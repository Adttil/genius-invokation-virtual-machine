[返回](../events.md)

# round_ended

双方均已宣告结束，当前回合已经关闭后的只读通知。

## 字段

无。

## 响应

固定响应程序会先于回合程序中后续的双方抽牌和下一次 [`start_round`](../instructions/start_round.md) 执行。广播期间，`table.state().round_number` 仍是刚关闭的回合编号，`table.state().active_player` 已经是下一回合先手，`table.state().first_ended == false`。
