[返回](../events.md)

# begin_action

驱动整个行动阶段：广播阶段开始、反复给予当前玩家行动机会、执行主动切人，以及处理双方宣告结束。

## 字段

无字段。流程阶段由栈顶 `stage_t` 表达，不保存在指令对象中。

## 栈

首次进入不要求特定栈顶布局。广播 `action_phase_started` 和 `before_action` 后，本指令压入等待行动 frame 并挂起：

```cpp
stack.top<
    detail::handler_id<cost_of_switch>[],
    cost_of_switch[],
    onpay_item<cost_of_switch>[],
    stack_count_t,
    action_argument,
    action_request,
    stage_t
>();
```

行动窗口枚举可切换出战的角色。handler 数组保存本行动窗口的费用响应者顺序；`cost_of_switch[]` 按可切换目标保存费用事件，`action_request.action_index` 是其从 0 开始的索引；`onpay_item[]` 是以 action 为行、handler 为列的行主序矩阵；`stack_count_t` 是确认支付后遍历选中 onpay 行的游标。

`stage_t` 位于物理栈顶，`action_request` 是它前面的输入槽；仅预览费用时只写 request。真正执行切人时外层同时写 `action_argument`，其中 `paid_dice` 是提交支付的骰子。当前切人目标由索引对应的 `cost_of_switch.target` 确定，不读取 `action_argument.target`。

进入固定 onpay 前，本指令临时压入：

```cpp
stack.top<
    detail::handler_id<cost_of_switch>,
    cost_effect_argument<cost_of_switch>,
    stage_t
>();
```

## 执行

1. 首次进入时推进 [`action_phase_started`](../events/action_phase_started.md) 广播；同一回合内后续行动机会不会重复广播。
2. 若上次战斗行动需要交接且对方尚未宣告结束，先切换 `active_player`。
3. 推进 [`before_action`](../events/before_action.md) 广播。
4. 创建行动等待 frame 并挂起。
5. 恢复后按 `action_request` 分支：
   - `none`：不处理行动，继续挂起。
   - `calculate_cost + switch_active`：重置选中的 [`cost_of_switch`](../events/cost_of_switch.md) 和对应 onpay 行，按 handler 顺序计算费用并缓存固定入口与 `cost_effect_argument`，随后将 request 复位为默认值并继续挂起；此时不执行 onpay。
   - `do_action_with_cost + switch_active`：使用本窗口中已经计算的选中费用事件和 onpay 行执行，不重新计算费用。
   - `do_action + switch_active`：现场计算费用并立即执行。
   - `do_action + declare_round_end`：弹出等待 frame，设置首次结束标记并推进 [`round_end_declared`](../events/round_end_declared.md) 广播。第一次声明后切换给对手并继续本指令；第二次声明后进入游戏规则程序中的下一条指令。
6. 确认切人后，按列进入选中 onpay 行中的非空固定入口。
7. 所有 onpay 完成后，从当前玩家扣除 `action_argument.paid_dice` 并广播 [`dice_removed`](../events/dice_removed.md)；零骰支付不广播。
8. 根据选中的 `cost_of_switch.target` 设置出战角色并广播 `active_character_changed`。战斗行动在对方尚未宣告结束时交接行动权；对方已宣告结束或本次为快速行动时，当前玩家继续行动，然后重新进入 `before_action`。

## 注意

通常回合程序应把 [`end_round`](end_round.md) 放在本指令之后。第二名玩家宣告结束时，`begin_action` 只进入下一固定槽，不在运行时生成或调度 `end_round`。

进入行动窗口前，当前玩家必须已有出战角色。行动索引和支付方案的合法性由外层检查。使用 `do_action_with_cost` 前，外层应先在同一窗口计算所选行动的费用。
