[返回](../events.md)

# action_phase_started

行动阶段已经开始的只读通知。

## 字段

无。

## 响应

响应程序可以执行行动阶段开始效果。广播期间，`table.state().active_player` 已是当前行动阶段第一名行动玩家，但 `begin_action` 尚未发出第一次 [`before_action`](before_action.md)；所有响应程序都会在该广播前结算完毕。
