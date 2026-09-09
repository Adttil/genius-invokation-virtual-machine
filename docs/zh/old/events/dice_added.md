[返回](../events.md)

# dice_added

指定骰子已经加入玩家资源的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `const player_id` | 获得骰子的玩家。 |
| `dice` | `const dice_counts` | 实际增加的组成。 |

## 响应

响应读取到增加后的 table，可安排后续效果。
