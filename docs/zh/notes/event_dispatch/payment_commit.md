[开发备忘](../../notes.md) / [事件分派](../event_dispatch.md)

# 费用预览、提交缓存与取消

这里记录行动费用为何需要延迟提交，以及修改报价、响应缓存和取消行为时需要维护的条件。主动切换与出牌的公开字段及用法见 [cost_of_switch](../../reference/definition/events/cost_of_switch.md)、[cost_of_card](../../reference/definition/events/cost_of_card.md) 和 [begin_action](../../reference/definition/commands/begin_action.md)。`onpay_context` 是费用事件到提交现场的现有映射，不是另一类事件。

## 为什么预览与提交隔离

费用预览需要计算每个候选行动的价格，但查看价格本身不应消耗减费次数或产生持久副作用。因此费用 handler 先修改报价事件，并记录本次响应实际产生的减费；需要确认后执行的效果，以已编译程序入口的形式暂存。玩家最终只选择一个候选，其他候选的暂存效果不会执行。

选择行动的公开现场为 `execution_state::action_selection`。[`calculate_switch_cost`](../../reference/executor/execution_view/action_selection/calculate_switch_cost.md) 在当前现场按切换候选索引同步重新报价，更新该候选的报价和 onpay 缓存，并返回报价的只读引用。它不接收调用方的随机源，不填写行动请求，也不推进执行器。`switch_active_character(index, paid)` 采用已经计算的费用；传入定义库和牌桌的重载先同步重新报价，成功后再填写选择。下一次 `step` 只执行已确认的费用结果，不再负责报价。

出牌沿用同一分工：`calculate_card_cost` 先以零骰子的快速行动构造 `card_cost_initialization`，只询问本牌定义，再依次调用 `cost_of_card` 响应并保存其 onpay。`play_card(card_index, paid, targets)` 采用出牌候选索引对应的已报价结果，带定义库与牌桌的重载同步重报；目标 span 默认为空，至多复制前两个 ID，缺项补 `std::monostate`。最终选择保存两个固定目标 ID，不记录目标数量。

费用 handler 保留统一的响应签名，但不得调用随机函数，违反此前提属于未定义行为。内部为这项签名提供的随机函数不连接对局随机源，其返回值不构成公开保证。报价及该行 onpay 结果在原处更新，不提供异常回滚。响应抛出异常后，该候选可能留下部分结果，调用方须重新完整报价成功后才能检查或采用；其他候选不受影响。重新报价的选择操作只有成功后才填写本次请求。

`card_cost_initialization` 与 `card_target_check` 都是本牌查询响应，只修改事件并返回空入口；仅目标检查响应可以省略，可打出的牌必须提供费用初始化和 `card_effect` 原效果响应。费用初始化、目标检查与 `cost_of_card` 使用内部零值随机函数适配统一签名；调用随机函数仍是文档规定的未定义行为，没有运行时检查机制。实际执行的 `card_effect` 可以使用本次 `step` 的随机源。

提交使用当次报价为该响应保存的参数，而不是重新计算一次减费。否则前后 table 状态的变化，或者其他响应已经产生的修改，可能使真正消耗的次数与先前采用的减费不一致。这个隔离也不等于数据库式事务：当前提交效果在扣除骰子之前执行，进入提交后已经完成的副作用不会自动回滚。不能把 onpay 理解为“扣款完成后才执行”。

费用预览只缓存入口，确认行动后才实际进入保存的非空 onpay 响应程序。当前不为进入响应程序设置独立观察停点，报价中的 event 修改也不等同于已经提交效果。外部观察范围见[执行观察与输入](../execution_observation.md)。

## 每个候选、每个响应分别保存结果

一次行动窗口保存费用响应者快照，并为每个候选分别保存报价和响应结果。缓存项属于“某个候选的一次 handler 调用”，包含返回入口及该次 `cost_effect_argument`，不能只按 handler 保存一份，也不能让不同候选共用同一份减费记录。

当前仍采用 C 个候选与 H 个响应者组成的 C×H 缓存矩阵。建立现场时不预先清零整个矩阵，实际报价才写入对应行，也不增加“已经报价”标志。未完整报价的行不得检查或采用，这由调用方保证。

切换候选只收集存活、非出战的己方角色，出牌候选只收集仍在手中的有效手牌，两份费用数组都保持紧凑。公开输入直接采用从零开始的候选索引，无需保存角色 ID、手牌 ID 到费用行的两份反向映射；`switch_target(target_index)` 与 `card_id(card_index)` 读取费用行中已有的目标 ID。公开通过 `switch_target_count()`、`card_count()` 提供数量，通过 `switch_cost(target_index)`、`card_cost(card_index)` 逐项读取费用，不暴露费用数组。不能把实体 ID 的 `index` 当作候选索引；进入下一个行动窗口后，调用方须重新取得候选。

同一候选的各个 handler 依次修改同一份报价，但提交参数不是跨 handler 累积的工作区。调用下一个 handler 前必须清空该参数，再单独保存这次调用写出的结果；重新计算候选时，也必须覆盖它原有的入口和参数，避免旧响应残留或把上一项减费计入下一项。

单个提交 context 只需要原响应实体的 ID 和这次响应保存的参数，其上的 activation 用于返回。它不借用原报价事件的工作区：后者在多次 handler 调用和重新报价时会改变，不能充当某次已采纳响应的稳定输入。需要增加提交信息时，应扩展明确保存的参数，而不是让响应程序深入读取原行动窗口的其他布局。activation 与栈借用的通用限制见[调用现场](execution_protocol.md)。

这些缓存属于当前行动窗口的临时现场，不进入定义库，也不成为长期牌桌状态。原有“候选行、handler 列”的安排表达的是上述隔离关系，不要求后续实现保留同一套数组或索引公式。

## 重新报价与响应者变化

候选目标在建立行动窗口时确定，`cost_of_switch::target` 为只读成员，费用 handler 不能改写它。每次重新报价都从同一目标的默认费用开始，不沿用上次减费结果。

当前重新报价只更新指定候选，不刷新其他候选，也不重新采样费用响应者。采用已计算费用时使用保留的报价；是否需要刷新其他候选，不能由缓存仍然存在来推断。

提交按先前保存的响应者顺序进入入口，`continue_switch_onpay` 不重新检查原 handler 是否仍有效。前一段提交效果可能使后面的响应者失效；后续消费指令的目标前提需要另行保证，不能照搬普通广播会跳过失效响应者的结论。

提交时不复查响应者是尚未调整的源码行为，不是对所有费用事件追加的规则保证。若要修改，需要同时检查报价采用的身份、参数和实际执行对象是否仍对应。

## 支付检查与提交

[`check_switch_payment`](../../reference/executor/execution_view/action_selection/check_switch_payment.md) 按候选索引读取已经完整报价的费用行，先检查所选骰子能否恰好满足 `fixed`、`same`、`any`，再检查当前行动玩家的持有数量，返回 `switch_payment_check_result`。它不计算费用、不提交、不执行 onpay，也不修改 table；提交接口不会自动调用它。

费用匹配算法由 [`action_selection` 视图](../../../../include/givm/executor/views/action_selection.hpp) 的私有静态函数 `payment_matches` 实现，随该视图的检查接口维护。

出牌的 `check_card_payment` 同样先检查费用匹配，再检查骰子持有量，并使用独立的 `card_payment_check_result`。`check_card_targets` 只询问本牌的 `card_target_check`，不依赖报价，也不验证支付；调用方传目标 span，视图最多采用前两个 ID，并将采用的数量写入事件的 `target_count`，空 span 也可以检查。响应只修改四态结果，区分无效、必须继续选择、可以完成也可以继续，以及已完成且不能继续。检查第二目标时以前一目标合法为前提，不重复检查第一目标，也不要求调用方实际调用过前一步。无响应时返回选择完成，不额外记录检查进度。两项检查都是独立可选查询，提交接口不会自动调用；输入合法性仍由上层保证。

行动窗口内部用 `std::variant<detail::round_end_selection, detail::switch_selection, detail::card_selection>` 保存最终选择：切换选择保存费用行与支付骰子，出牌选择还保存两个目标，结束声明不带参数。选择种类与对应参数始终成组，公开的 `action_argument`、`action_target` 不进入行动帧，也不保存“未选择”标记。上层须先通过 `play_card`、`switch_active_character` 或 `declare_round_end` 提供输入，再调用 `step`；查询与检查不算输入，库不检查未输入就推进的非法调用。首备选仅用于内部默认构造，不构成默认宣布结束的公开约定。默认构造是否平凡与平凡可复制是不同要求，现有 `frame_stack` 要求后者，以便复制和搬移。

提交仍先执行所选行的 onpay，再扣除支付骰子。支付总数为零时跳过扣骰及 `dice_removed` 广播，直接进入切换或出牌本体；这不会跳过此前应执行的 onpay。

出牌在 onpay 之前先标记选中手牌删除，使其离开普通手牌遍历与广播；原 ID 在清理前仍可读取。随后执行缓存 onpay、扣骰与骰子变化广播，再广播 `card_will_be_played`。未反制时单独调用本牌 `card_effect` 并执行原效果，最后无论是否反制都广播 `card_played`。已删除牌自身仍通过保存的牌 ID 和定义执行原效果，普通广播则跳过它；反制不会撤销此前的离手与支付。

## 取消与缓存的寿命

上层保留当前现场、改选其他候选或重新报价时，不会执行未提交的效果。未推进时已经计算的候选仍可保留；重新报价会覆盖对应候选的旧结果。正常结束该行动窗口时丢弃全部候选和缓存，宣布回合结束也不会提交此前的报价效果。

进入提交后，暂停推进并不撤销已经发生的副作用；当前也没有独立的提交回滚机制。缓存只说明此前某次报价的结果，不证明支付已经经过完整合法性检查。支付合法性仍由调用方按[行动接口](../../reference/definition/commands/begin_action.md)保证。

行动现场被弹出或执行器重新开始后，旧缓存和从中取得的引用都不能沿用；栈扩容也可能更早使借用失效。进入终局后旧现场被逻辑废弃，即使字节仍在栈中，也不能继续解释或借用这份缓存。

`calculate_switch_cost` 与 `calculate_card_cost` 返回的引用分别与 `switch_cost(target_index)`、`card_cost(card_index)` 观察同一份报价，不提供独立快照。同一行动现场内重新计算该候选会更新引用所见的结果；公开借用仍只保持到下一次推进或重建现场之前。

旧 `calculating_card_payment`、`calculating_skill_payment`、`calculating_switch_payment` 所讨论的 table payment 槽位与 `on_pay` 属于另一批历史方案，见[旧 payment 设计](resource_events.md)。它们不能代替这里按报价保存响应结果的模型。

实现核对位置：[begin_action.hpp](../../../../include/givm/executor/commands/begin_action.hpp)。
