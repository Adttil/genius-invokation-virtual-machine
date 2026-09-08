[返回](../events.md)

# hand_card_created

一个全新的手牌实体已经创建的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 新创建的手牌。 |

## 响应

用于“获得生成的牌后”效果。它不用于从牌库抽到手牌，后者广播 [`card_drawn`](card_drawn.md)。
