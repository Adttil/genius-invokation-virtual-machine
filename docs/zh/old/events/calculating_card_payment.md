[返回](../events.md)

# calculating_card_payment

计算一张手牌 payment 时发布的事件。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 正在报价的手牌。 |

## 响应

payment 保存在该手牌对应的 table 槽位。响应通过 command 修改费用、速度或 `on_pay`，后一个响应读取前一个响应完成后的最新槽位。
