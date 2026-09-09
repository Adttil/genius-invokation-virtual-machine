[返回](../events.md)

# dice_converted

骰子转换已经完成的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `const player_id` | 目标玩家。 |
| `from` | `const elemental_dice` | 原骰子类型。 |
| `to` | `const elemental_dice` | 新骰子类型。 |
| `count` | `const std::uint8_t` | 实际转换数量。 |

## 响应

用于转换完成后的效果，不能修改已完成的转换。
