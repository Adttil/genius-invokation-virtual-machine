[返回](../events.md)

# before_action

当前行动玩家即将选择行动的只读通知。

## 字段

无。

## 响应

响应者通过 `table.state().active_player` 得知即将行动的玩家。若 `table.state().first_ended == true`，表示本回合已有玩家宣告结束；在正常行动机会中，首个结束玩家是 `other_player(table.state().active_player)`。
