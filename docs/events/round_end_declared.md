[返回](../events.md)

# round_end_declared

当前行动玩家已经宣告结束本回合的只读通知。

## 字段

无。

## 响应

响应者通过 `table.state().active_player` 得知宣告者。若这是本回合第一次宣告结束，`begin_action` 会在广播前先设置 `table.state().first_ended = true`；广播期间 `active_player` 仍保持为宣告者，广播结束后才切换给对手。第二次声明广播完成后，`begin_action` 进入回合程序中的下一条 `end_round`。
