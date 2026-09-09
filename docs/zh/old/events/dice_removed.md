[返回](../events.md)

# dice_removed

指定骰子已经从玩家资源移除的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `const player_id` | 失去骰子的玩家。 |
| `dice` | `const dice_counts` | 实际移除的组成。 |

## 响应

[`begin_action`](../instructions/begin_action.md) 在主动切人的所有 onpay 程序完成后扣除 `action_argument.paid_dice`；若实际支付的骰子总数非零，随后广播本事件，全部响应完成后才设置新的出战角色并广播 [`active_character_changed`](active_character_changed.md)。支付零个骰子时跳过本事件。

响应读取到扣骰后的 table。其他资源移除是否广播仍由发起该变化的指令规定；例如 `start_round` 清空骰子时不广播本事件。
