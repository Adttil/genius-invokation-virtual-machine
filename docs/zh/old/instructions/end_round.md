[返回](../events.md)

# end_round

关闭当前回合。

## 字段

无。

## 栈

本指令入口不要求额外栈输入。执行期间会临时压入 `round_ended` 广播 frame；广播完整结算后弹出该 frame，恢复入口栈形状。

## 执行

1. 要求入口处 `table.state().active_player` 是第二个宣告结束的玩家。先令 `table.state().active_player = other_player(table.state().active_player)`，使其变为下一回合先手玩家。
2. 设置 `table.state().first_ended = false`。
3. 推进 [`round_ended`](../events/round_ended.md) `{}` 广播。
4. 进入游戏规则程序中的下一条指令。

## 注意

`round_ended` 响应期间，`table.state().round_number` 仍是刚关闭的回合编号，`active_player` 已经是下一回合先手，`first_ended` 已经清空。

本指令不隐式抽牌，也不生成 [`start_round`](start_round.md)。需要回合结束抽牌的回合程序应在本指令之后显式放置 [`draw_cards`](draw_cards.md)；回合程序结束后会重新进入下一回合。
