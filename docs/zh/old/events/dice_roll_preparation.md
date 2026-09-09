[返回](../events.md)

# dice_roll_preparation

每回合初始投骰取得随机结果前，用于收集双方固定骰子和重投次数的可修改工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `count` | `const std::uint32_t` | 每名玩家本次初始投骰的骰子总数。 |
| `fixed_dice` | `std::array<dice_counts, 2>` | 双方不需要随机生成的固定骰子，默认均为空，可修改。 |
| `reroll_count` | `std::array<std::uint32_t, 2>` | 双方本回合投掷阶段可用的重投次数，默认均为 1。 |

## 响应

- “若干骰子总是投出某元素”修改对应玩家的 `fixed_dice[player.index]`。
- “投掷阶段：获得额外一次重投机会”等效果修改对应玩家的 `reroll_count[player.index]`。
- 响应结束后，[`start_dice_roll_phase`](../instructions/start_dice_roll_phase.md) 会完成首次随机投骰，预发双方重投随机池，并在需要时继续管理重投阶段。
- 指令在本事件结算后才生成初始骰子和重投随机池；handler 自身仍可消费传入的随机源。核心流程不再提供单独的初始投骰后通知事件。
- 每名玩家都必须满足 `fixed_dice[player.index].total() <= count`。
