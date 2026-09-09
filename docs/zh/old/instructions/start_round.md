[返回](../events.md)

# start_round

开始下一个回合。

## 字段

| 字段 | 类型 | 默认值 | 含义 |
| --- | --- | ---: | --- |
| `max_rounds` | `std::uint32_t` | 14 | 允许开始的最大回合编号。 |

## 栈

除执行器维护的公共 `stage_t` 后缀外，本指令不要求额外栈输入，也不创建临时 frame。

## 执行

1. 令 `next_round = table.state().round_number + 1`。
2. 若 `next_round > max_rounds`，以 `both_loss` 结束对局并返回 `false`，使外层能通过 `executor.status()` 观察结果。
3. 否则，将 `table.state().round_number` 设为 `next_round`，并清空双方已有骰子。
4. 进入游戏规则程序中的下一条指令。

## 注意

`start_round` 不隐式生成后续流程，也不修改 `active_player` 或 `first_ended`。投骰、战斗开始和行动阶段由调用者放在回合程序中的后续固定指令表达；通常把本指令放在回合程序开头，使回合程序每次循环时先推进回合编号。
