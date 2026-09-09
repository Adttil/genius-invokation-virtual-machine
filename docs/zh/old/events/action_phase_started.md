[返回](../events.md)

# action_phase_started

行动阶段已经开始的只读通知。

## 字段

无。

## 响应

响应程序可以执行行动阶段开始效果。进入 [`begin_action`](../instructions/begin_action.md) 前，调用方规则程序应使 `table.state().active_player` 指向当前行动阶段第一名行动玩家；`begin_action` 不在本事件前另行确定先手。所有响应程序都会在本次 `begin_action` 第一次 [`before_action`](before_action.md) 广播前结算完毕。
