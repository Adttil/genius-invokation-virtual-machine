# 指令、栈与事件系统

本页规定规则拓展者使用公开指令、实现事件响应时涉及的公共执行协议，包括重入、activation、默认广播和 onpay 栈 ABI。上层如何取指、写入输入和记录动画见 [Table、Executor 与外层观察](table-vm-and-input.md)；具体字段与领域执行顺序见 [当前指令与事件目录](events.md)。

## 指令协议

具体指令提供只读执行入口：

```cpp
bool execute(card_table& table, execution_context& context, random_fn& random) const;
```

指令对象保存编译时已确定的操作数，不保存可变执行阶段。它必须满足公开擦除边界的复制、析构、大小和对齐要求。大型临时数据、事件、输入与重入阶段放在 executor stack；跨指令持续存在的规则状态放在 table。

公开规则指令应满足干净完成约束：当它彻底完成并进入下一条规则指令时，stack 恢复到进入该指令时的形状。指令执行期间可以压入临时 frame、进入响应程序或挂起。进入终局会保留现场，不要求恢复入口形状。

当前活动 frame 以 `stage_t` 结尾。stage 没有独立类型信息，必须与当前指令类型共同解释。具体指令页面必须写明每个可观察阶段的完整 frame 或可供外层访问的固定后缀。

## 执行控制

`execution_context` 向指令提供受限控制接口：

```cpp
context.stack();
context.enter_next();
context.yield();
context.yield_next();
context.enter(program_entry<Context>);
context.end_game(result);
```

- `enter_next()` 清零当前 stage，并进入固定程序中的下一条指令。
- `yield()` 保留当前执行位置和 stage，本次执行返回 `false`；指令必须已经把 stack 调整为下一次重入所需形状。
- `yield_next()` 清零当前 stage，进入下一条指令后返回 `false`，用于提供独立观察点。
- `enter(entry)` 压入 activation frame，并进入同一固定程序中的非空强类型入口。
- `end_game(result)` 以指定结果结束对局并返回 `false`，保留当前 stack。

`bool` 返回值表示 executor 是否可以继续自动推进。`false` 不保证执行位置保持不变，因此上层必须在调用返回后重新读取位置和 stack。

## Activation ABI

进入可返回的 `program_entry<Context>` 时，stack 顶部增加：

```cpp
frame<execution_context::return_info, stage_t>
```

`return_info::return_position` 记录调用者的恢复位置，activation stage 初始为零。响应程序结束时弹出整个 activation frame，并从该位置继续原结算过程。

Context-bound 指令通常需要同时读取 activation frame 和下方的调用 Context。例如普通事件响应读取：

```cpp
auto&& [broadcast, activation] = stack.top<
    frame<handler_id<E>[], stack_count_t, E, handler_id<E>, stage_t>,
    frame<execution_context::return_info, stage_t>
>();
```

程序段中的每条干净指令完成时都应保留下方 Context frame 和 activation frame，使下一条响应指令看到相同 ABI。程序返回后，调用者继续解释原来的 Context frame。

## Stack 访问

一次 `push(...)` 创建一个 frame。`top` 返回非持有 `frame_view`；后续 `push`、`pop`、扩容、移动或交换 stack 后，已有 view 和从中借出的引用都可能失效。

动态数组、固定后缀访问、多 frame 访问和完整 frame 弹出规则见 [Executor Stack](stack.md)。这些规则也是外层观察指令状态的基础。

## 默认广播 ABI

默认广播开始时立即采样本次响应者，并压入完整广播 frame：

```cpp
stack.top<
    handler_id<E>[],
    stack_count_t,
    E,
    handler_id<E>,
    stage_t
>();
```

各槽位语义为：

- `handler_id<E>[]`：广播开始时的响应者快照；
- `stack_count_t`：下一个待调用响应者的游标；
- `E`：可由 handler 修改的事件对象；
- 第二个 `handler_id<E>`：当前正在调用的响应者；
- `stage_t`：产生本次广播的指令阶段。

默认采样按 table 的规则顺序遍历实体，只加入 `can_handle<E>()` 为真的有效实体。广播期间新建的实体不加入本次快照；已经进入快照但随后失效的实体在轮到时跳过。

推进器在调用 handler 前写入当前响应者，并先推进游标。这样 handler 返回的固定程序完成后，产生事件的指令可以从下一响应者继续，而不会重复调用当前项。

## Handler 与事件修改

默认 handler 接口接收 definition data、自身 entity view、可修改 event、只读 table 和随机函数，并返回 `handler_program_entry_t<E>`。handler 可以执行普通 C++ 计算，因此按层数、生命、费用结果或其他运行期状态修改事件不需要额外表达式语言。

- 返回空入口：不进入响应程序，保留 handler 对 event 的修改。
- 返回普通入口：立即进入固定响应程序，返回后继续广播。
- 通过 `program_entry` 的具名结果工厂结束对局，并保留当前广播和 activation 现场。

handler 不能通过收到的 `const card_table&` 直接修改持久状态。需要产生副作用时，由返回的固定程序执行相应指令。

产生事件的领域指令拥有整个事件事务：它准备 frame、推进响应者、读取最终 event、执行必要后处理并弹出 frame。例如 `deal_damage` 自己组织伤害计算、伤害效果、扣血、元素附着、伤害后广播和胜负检查。通用广播工具不额外猜测事件完成后的领域行为。

## Context-bound 指令

不依赖调用事件的指令不声明 `context_type`。需要直接读取某种事件 frame 的指令声明：

```cpp
using context_type = E;
```

定义源把该指令加入 `E` 程序段时会静态检查兼容性。运行时不再检查 Context，也不在擦除指令中保存 Context 标签。

例如 `absorb_damage_by_count` 只能位于 `damage_effect` 响应程序。它从广播 frame 读取伤害事件和当前 handler，按规则同时修改伤害值与当前实体计数；不需要在指令对象中复制运行期目标或层数。

## 费用与 onpay

费用事件的 handler 仍使用普通五参数形状，但返回 onpay Context 的入口，例如：

```cpp
program_entry<onpay_context<cost_of_switch>>
```

费用预览只调用 handler，不立即进入返回程序。行动指令为每个候选 action、每个 handler 保存入口以及该次 `cost_effect_argument<CostEvent>`。玩家确认后，只遍历选中 action 的 onpay 项，并按原 handler 顺序进入非空入口。

进入单个 onpay 程序前，调用者压入：

```cpp
frame<
    handler_id<CostEvent>,
    cost_effect_argument<CostEvent>,
    stage_t
>
```

onpay Context 下的指令只依赖这个最小提交 frame 和其上的 activation frame。当前七圣召唤规则只需要从 `cost_effect_argument` 读取该 handler 实际减少的骰子数；以后增加费用提交信息时，优先扩展该参数，而不是让 onpay 深入读取已经结束的费用预览栈。

完整行动等待 frame、费用矩阵、付款和行动本体顺序由 [`begin_action`](instructions/begin_action.md) 规定；具体费用事件字段由对应事件页面规定。

## 非默认事件协议

并非所有事件都必须通过全局默认广播。领域指令可以准备自己的响应者集合，或直接调用单个 definition handler，但必须在具体指令和事件文档中明确采样范围、调用顺序、返回入口是否进入以及完整栈 ABI。

`character_initialization` 是当前单目标特例。`enter_character` 初始化刚创建的角色，`initialize_characters` 依次初始化已经装入 table 的角色；二者都只调用当前角色自己的 handler，约定其只修改局部 event 并返回空入口，当前不进入响应程序。这个限制目前是公开协议约定，未来若增加静态限制，需要保持现行语义。

## 规范归属

- 本页规定所有指令和事件共享的执行、activation、广播与 onpay 协议。
- [Executor Stack](stack.md) 规定栈容器接口和 view 生命周期。
- [当前指令与事件目录](events.md) 及其子页面规定每个类型的字段、默认值、领域时序、挂起点和栈 ABI。
- [固定程序模型](fixed-program-design.md) 规定入口、Context 和程序表达能力。
- [Table、Executor 与外层观察](table-vm-and-input.md) 规定上层驱动、取指和输入方式。

[返回当前指令与事件目录](events.md)
