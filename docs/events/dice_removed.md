[返回](../events.md)

# dice_removed

指定骰子已经从玩家资源移除的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `const player_id` | 失去骰子的玩家。 |
| `dice` | `const dice_counts` | 实际移除的组成。 |

## 响应

支付和普通移除是否广播由对应 command 的规则决定。
