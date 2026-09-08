[返回](../events.md)

# battle_started

首个回合投掷阶段完成后、行动阶段开始前的战斗开始时机。

## 字段

无。

## 响应

用于“战斗开始时”效果。默认流程中，首次投骰和双方重投都已经完成；行动阶段尚未开始。响应完整结算后，游戏规则程序继续进入 [`begin_action`](../instructions/begin_action.md)。
