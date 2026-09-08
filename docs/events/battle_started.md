[返回](../events.md)

# battle_started

由 [`start_battle`](../instructions/start_battle.md) 广播的战斗开始时机。

## 字段

无。

## 响应

用于“战斗开始时”效果。响应完整结算后，`start_battle` 进入调用方规则程序的下一条指令。

七圣召唤式流程将 `start_battle` 放在首次投骰及双方重投完成之后、首次 [`begin_action`](../instructions/begin_action.md) 之前。这一顺序由调用方显式提供的程序安排，事件本身不检查投骰或行动阶段状态。
