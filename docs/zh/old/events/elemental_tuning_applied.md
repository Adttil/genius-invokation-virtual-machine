[返回](../events.md)

# elemental_tuning_applied

元素调和已经完成的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 已弃置手牌的历史 ID。 |
| `from` | `const elemental_dice` | 原骰子类型。 |
| `to` | `const elemental_dice` | 目标骰子类型。 |

## 响应

响应读取到弃牌和转骰后的状态。
