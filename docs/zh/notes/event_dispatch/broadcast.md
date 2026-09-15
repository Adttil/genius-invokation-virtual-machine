[开发备忘](../../notes.md) / [事件分派](../event_dispatch.md)

# 默认广播工具与事件事务

默认广播是一组可替换的实现工具，不是所有事件、指令必须采用的广播规则。本页记录这组工具的采样顺序、frame 结构、响应推进和单目标例外。

## 广播辅助工具与 frame ABI

默认广播只是供领域指令复用的辅助工具，不规定所有事件统一采用某种响应顺序或范围。不同事件，以及同一事件在不同指令中的广播，都可以采用各自的响应顺序、实体子集或单实体调用。本节描述辅助工具的行为；具体广播规则由发起广播的指令和事件协议规定。

下文及具体指令页面使用的响应者 ID 类型为 `givm::detail::handler_id<E>`，定义在 `<givm/executor/broadcast.hpp>`。它按事件 `E` 的订阅关系组合相应实体 ID；读取广播 frame 时必须使用这一类型。示例中的其他未限定名称位于 `givm` 命名空间。

默认广播在准备 frame 时立即采样本次响应者，并压入完整广播 frame：

```cpp
stack.top<
    detail::handler_id<E>[],
    stack_count_t,
    E,
    detail::handler_id<E>
>();
```

各槽位语义为：

- `detail::handler_id<E>[]`：frame 准备时的响应者快照；
- `stack_count_t`：下一个待调用响应者的游标；
- `E`：可由 handler 修改的事件对象；
- 第二个 `detail::handler_id<E>`：当前正在调用的响应者。

恢复时，执行位置指向广播推进入口，广播 frame 不再保存 stage。

辅助工具目前采样时先遍历玩家 0，再遍历玩家 1，不根据 `active_player` 调换顺序。对每名玩家，依次遍历：

1. 手牌，每张牌之后紧接该牌的附着状态；
2. 牌堆中的牌，按逻辑顺序从底到顶，每张牌之后紧接该牌的附着状态；
3. 支援、召唤物、战斗状态；
4. 角色，每名角色之后紧接其技能，再接其附件。

各区域及子实体使用对应遍历接口的顺序，执行侧根据实体定义 ID 查询显式传入的定义库，只加入具有对应响应的有效实体。frame 准备后新建的实体不加入其快照；已经进入快照但随后失效的实体在轮到时跳过。每个广播 frame 分别保存准备时的快照；前一次响应新建的实体只可能被尚未准备的后续广播采样，不会加入已经预备好的快照。领域指令采用非默认采样范围或顺序时，以其自身文档为准。

推进器在调用 handler 前写入当前响应者，并先推进游标。这样 handler 返回的固定程序完成后，产生事件的指令可以从下一响应者继续，而不会重复调用当前项。

## Handler 与事件修改

默认 handler 接口接收 definition data、自身 entity view、可修改 event、只读 table 和随机函数，并返回 `handler_program_entry_t<E>`。handler 可以执行普通 C++ 计算，因此按层数、生命、费用结果或其他运行期状态修改事件不需要额外表达式语言。

- 返回空入口：不进入响应程序，保留 handler 对 event 的修改。
- 返回普通入口：进入固定响应程序，程序返回后继续广播。
- 响应程序执行 `end_game` 时，栈顶保存结果，旧广播和 activation 被逻辑废弃，不再继续广播；入口本身不再有独立的终局种类。

handler 不能通过收到的 `const table&` 直接修改持久状态。需要产生副作用时，由返回的固定程序执行相应指令。

产生事件的领域指令拥有整个事件事务：它准备 frame、推进响应者、读取最终 event、执行必要后处理并弹出 frame。例如 `deal_damage` 自己组织伤害计算、伤害效果、扣血、元素附着、伤害后广播和胜负检查。通用广播工具不额外猜测事件完成后的领域行为。

## 非默认事件协议

领域指令可以准备自己的响应者集合，或直接调用单个 definition handler。具体指令和事件的公开约定说明响应范围、调用顺序及返回入口的处理；完整内部帧由源码维护。采用辅助工具的默认遍历不构成所有广播都必须遵循的规则。

`character_initialization` 是当前单目标特例。`enter_character` 初始化刚创建的角色，`initialize_characters` 依次初始化已经装入 table 的角色；二者都只调用当前角色自己的 handler，约定其只修改局部 event 并返回空入口，当前不进入响应程序。这个限制目前是公开协议约定，未来若增加静态限制，需要保持现行语义。

## 与当前实现逐项核对

源码为 [`broadcast.hpp`](../../../../include/givm/executor/broadcast.hpp)。`handler_id<E>` 的 variant 不是手写通用实体引用：它依次展开 `definition_types`、`views_of_definition`，仅纳入 `subscribed_events<View>` 包含事件 `E` 的 view 所对应的实体 ID。实体身份为何区分区域，见[实体身份与区域](../entity_identity.md)。

`prepare_broadcast` 在调用时完成整个目标列表采样；`continue_broadcast` 不重新采样。后者读取 `targets/cursor/event/current_handler`，在调用前执行等价于 `current_handler = targets[cursor++]` 的操作。快照阶段先根据定义 ID 筛选定义库中的响应能力，调用阶段的 `try_handle` 只检查实体是否仍有效，不重新做一次全体订阅扫描。

`current_handler` 保存当前响应者自身（self）的身份，不是事件的 `target`。事件指向的受伤角色可以与响应的护盾、支援或卡牌不同；`absorb_damage_by_count` 正是从这一槽取得应扣计数的实体。进入响应子程序后它留在下方广播 frame 中，activation 上的指令不需要把 self 复制进自身固定操作数。初始压帧时该槽只是默认构造，推进器写入当前项之后才具有这个“当前响应者”的意义。

`continue_broadcast` 的结果区分“本轮广播已走完”与“刚进入响应程序”。快照中的全部响应者走完后，领域指令才继续事件后处理和 `pop_broadcast`；取得非空入口时直接进入响应。该辅助函数的返回不代表一条指令只执行了一部分。

广播辅助工具不负责外部观察停点，也不在 activation 上方另压实体身份或专用观察阶段。当前观察范围与撤回通用效果入口通知的理由见[执行观察与输入](../execution_observation.md)。

普通广播中的已失效实体被跳过，不等同于可任意清理或重排底层存储：快照保存的是 ID，跨 `clean_up()` 使用 ID 的限制仍然适用。不能拿 `try_handle` 的无效检查当作悬空对象、越界 ID 或清理后旧 ID 的保护。

### 多个预备广播的快照边界

[`select_active_character_both`](../../../../include/givm/executor/instructions/select_active_character_both.hpp) 在收齐双方选择、同时写入出战角色之后，先压玩家 1 的广播，再压玩家 0 的广播。观察模式先报告初选完成，下一次推进才用仍保留的原选择帧准备这两个广播；外部从牌桌读取双方结果。栈顶先处理玩家 0 的通知，但两个响应者快照此前都已建立。因此玩家 0 的响应新建的实体不会进入已经预备好的玩家 1 快照。这个例子解释为什么不能把“每个事件有自己的快照”误写成“前一个广播结束后才采样下一个”。

相比之下，`draw_cards`、单方 `replace_cards` 和元素反应后的通知，是推进到后一个广播时才重新调用 `prepare_broadcast`；后一个广播可采样之前响应创建的实体。

### 初始化特例的额外前提

[`enter_character`](../../../../include/givm/executor/instructions/enter_character.hpp) 与 [`initialize_characters`](../../../../include/givm/executor/instructions/initialize_characters.hpp) 直接调用当前角色定义的 `handle<character_initialization>`，没有先调用 `can_handle` 检查。角色定义必须提供这个 handler。事件是函数内的局部值，调用后把 `event.state` 写回角色，返回入口被 `(void)` 丢弃；即使 handler 返回含 end_game 的程序入口也不会由这两条指令进入。旧文建议返回 null 保持这个事实明确。

旧文把“将来可增加静态限制但应保持现有语义”作为设计余地。这里仍保留该余地，不把它误记为已有静态限制。
