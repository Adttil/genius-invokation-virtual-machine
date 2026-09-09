[返回](../events.md)

# round_ended

由 [`end_round`](../instructions/end_round.md) 广播的回合结束只读通知；七圣召唤式规则程序在双方均已宣告结束后执行该指令。

## 字段

无。

## 响应

[`end_round`](../instructions/end_round.md) 广播前将 `table.state().active_player` 切换为对手，并设置 `table.state().first_ended = false`；它不改变 `round_number`。当该指令紧接完成双方宣告的 `begin_action` 时，广播中的 `active_player` 因而是本回合首先宣告结束的一方，即下一回合先手。

全部响应完成后才继续规则程序。后续双方抽牌及下一次 [`start_round`](../instructions/start_round.md) 的位置由调用方显式提供的程序决定。
