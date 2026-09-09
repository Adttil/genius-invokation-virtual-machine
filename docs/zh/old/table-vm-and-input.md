# Table、Executor 与外层观察

核心只保存规则执行所需的数据，不保存界面文字、动画描述、候选项 UI 或日志。上层在一份 `definition_library` 下组合 `card_table` 与 `executor`，通过当前指令、table 和 stack 观察规则进程并提供输入。

## 对象职责

`definition_library` 是可共享的不可变游戏规则。`card_table` 引用相应 definition library，并保存实体、资源、行动上下文和回合信息。`executor` 保存当前执行位置与执行栈。后两者共同构成一局游戏的可变规则状态，definition library 本身不属于该状态。

执行时，executor 通过参数 table 找到相应定义库和当前指令。definition library 的生命周期必须覆盖 table；挂起或分支模拟时复制匹配的 table 与 executor，并让副本共享同一定义库。持久化时可以另外记录足以重建定义库的构建信息，而不是把编译库误作可变游戏状态。

游戏结果不是 table 常驻字段。上层通过 `executor::status()` 查询；进入终局会保留 stack，便于记录最终结算现场。`executor::clear()` 只清空 stack，保留当前执行位置和游戏结果；重新使用 executor 时应调用 `enter_entry(definition_library)`。

## Executor 接口

```cpp
execution_position position() const noexcept;
game_result status() const noexcept;
void clear() noexcept;
void enter_entry(const definition_library&);

template<class TRandom>
bool execute_next(card_table&, TRandom&);

auto& stack() noexcept;
```

默认构造的 executor 尚未开始执行。`enter_entry(library)` 清空已有 stack、建立初始执行 frame，并从该 definition library 的 `entry()` 开始一次新的执行。它不会保存 definition library 的引用；在下一次 `enter_entry()` 前，调用方必须使用引用同一 definition library 的 table 推进和观察该 executor。

`execute_next()` 完整执行一次当前公开指令。一次执行是同步且不可打断的，返回时本次执行已经结束。指令对象不保存运行时状态，而是根据 table 和 stack 中的信息决定本次操作与后续执行位置；`stage_t` 是栈上的结算状态。后续位置可以仍是该指令，此时下一次调用会根据新的栈状态再次完整执行它，这就是自重入。

返回 `false` 表示外层应暂停自动推进，不表示本次指令执行在中途被打断。对局结果由 `status()` 判断，返回 `true` 本身不保证对局尚未结束。调用方应在返回后重新读取 `status()` 和 `position()`，不能根据返回值推断执行位置是否变化。

具体指令通过 `execution_context` 明确选择进入下一条、保留当前位置挂起、在下一条挂起、进入响应程序或结束对局。正常流程必须通过 `execute_next()` 推进；程序段之间的连接不会成为独立的公开执行步骤。

## 指令观察

开始执行后，当 `status()` 为 `no_result` 时，`executor::position()` 标识的必定是一条公开指令。外层通过 table 所引用的 definition library 观察该位置：

```cpp
decltype(auto) instruction =
    table.definition_library().instruction(executor.position());
```

返回对象的具体类型以及按值或按引用返回均不是公开接口的一部分。调用者必须使用 `const auto&`、`auto&&` 或 `decltype(auto)` 接收，不能用 `auto` 或 `const auto` 要求结果可复制。

返回结果公开以下能力：

```cpp
template<class TInstruction>
bool is() const noexcept;

template<class TInstruction>
decltype(auto) as() const;

const void* type_index() const noexcept;

template<class TInstruction>
inline constexpr auto instruction_type_index = /* 对应类型索引 */;

bool execute(card_table&, execution_context&, random_fn&) const;
```

`is<T>()` 为真是调用 `as<T>()` 的前置条件。`as<T>()` 提供对该指令 payload 的只读访问，但其准确返回类别不是契约；调用方不应复制未保证可复制的 view，也不应把借用结果长期保存。

`type_index()` 可以与 `instruction_type_index<T>` 比较，供不需要读取字段的观察器分类。类型索引只用于当前 C++ 程序中的身份比较，不是持久化格式或跨进程数值 ABI。

取指结果还公开底层 `execute(...)` 转发能力，但它不代替 executor 的流程控制。一般对局、回放和模拟仍调用 `executor::execute_next()`。上层通过上述 `position()` 取指流程只能得到公开指令。

## Stack 观察

executor stack 保存事件、输入槽、广播游标、activation frame 和其他临时结算状态。它不保存运行期类型信息，也不标注哪些对象可由外层写入。

当前活动 frame 通常以 `stage_t` 结尾；当前指令类型和 stage 共同确定栈布局。每个需要观察的具体指令页面必须说明其完整 frame、可访问后缀、输入槽和挂起条件。

`frame_view` 是非持有 view。任何可能改变 stack 存储的操作后都必须重新获取。完整访问规则见 [Executor Stack](stack.md)。

## 外层输入

外层输入的一般流程是：

1. 执行直到 `execute_next()` 返回 `false`，或 `status()` 不再是 `game_result::no_result`。
2. 若 `status()` 已有结果，进入终局观察流程。
3. 否则重新通过 `table.definition_library().instruction(executor.position())` 识别当前指令。
4. 按该指令文档读取 table 与 stack，构造候选项并完成合法性检查。
5. 只写入文档指定的完整输入对象槽，再继续执行。

输入对象位于相应 frame 的固定部分尾部，通常紧邻 `stage_t` 之前；外层不改写 stage。隐藏结算状态、动态数组、广播游标和预发随机池仍由核心指令维护。

例如，开局换牌和重投使用 `selector`，双方出战角色选择使用 `character_id`，行动窗口使用 `action_argument` 与 `action_request`。准确布局以 [`replace_cards_both`](instructions/replace_cards_both.md)、[`start_dice_roll_phase`](instructions/start_dice_roll_phase.md)、[`select_active_character_both`](instructions/select_active_character_both.md) 和 [`begin_action`](instructions/begin_action.md) 为准。

核心假定提交给执行指令的输入已经合法。候选项生成和合法性检查属于上层接口，不由 stack 自动验证。

## 运行到挂起

```cpp
while(
    executor.status() == game_result::no_result
    && executor.execute_next(table, random)
)
{
}
```

循环结束后重新读取 `status()`。若仍为 `no_result`，当前执行位置、指令和 stack 共同描述等待中的输入或观察点。

随机函数的记录、回放和预发语义见 [随机输入](random-input.md)。

## 日志与动画

核心不提供日志回调。外层可以在每次 `execute_next()` 前观察当前指令及其公开字段，在执行后比较 table 与 stack，从而生成规则日志、动画任务或调试记录。

需要跨执行步骤保存的信息必须复制为上层自己的稳定数据。取指结果、`as<T>()` 返回值、`frame_view` 和实体 view 都可能因下一次执行、stack 扩容或 table cleanup 失效，不应直接保存在长期展示任务中。

当 `execute_next()` 返回 `false` 时，执行位置可能已经变化。记录器应重新取指并读取 stack，分别记录“刚执行的规则步骤”和“实际到达的挂起点”。

## Cleanup Safe Point

实体离场先标记 invalid。`table.clean_up()` 会压缩实体容器，使旧实体 ID、entity/view 以及外层保存的相关位置失效。

核心当前不提供通用 safe-point 查询。调用者必须根据具体领域流程确认活动 stack、日志和展示任务不再引用待压缩实体。终局时常用顺序是：先观察结果和现场，再 `executor.clear()`，最后执行 table cleanup。非终局挂起期间通常仍存在活动 frame，不能仅因 `execute_next()` 返回 `false` 就清理 table。

[返回架构总览](architecture.md)
