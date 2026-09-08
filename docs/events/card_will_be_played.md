[返回](../events.md)

# card_will_be_played

支付提交后、牌实际生效前的可修改工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 要打出的手牌。 |
| `definition_id` | `const definition_id<card_definition>` | 已捕获的牌定义。 |
| `target` | `std::variant<std::monostate, character_id, support_id, summon_id>` | 最终具体目标，可修改。 |
| `speed` | `action_speed` | 行动速度，可修改。 |
| `effect_cancelled` | `bool` | 是否仅无效化牌定义效果。 |

## 响应

无效化效果设置 `effect_cancelled`；改目标和改速度直接修改字段。该标志不会退费或阻止牌离开手牌。实体消耗等副作用由 handler 返回的固定响应程序完成。
