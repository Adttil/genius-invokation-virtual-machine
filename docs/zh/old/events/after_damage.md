[返回](../events.md)

# after_damage

角色生命值已经扣除，且该伤害携带的元素附着/反应流程已经处理后的时机。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `const std::variant<hand_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 伤害来源。 |
| `target` | `const character_id` | 受伤角色。 |
| `value` | `const std::uint32_t` | `damage_effect` 响应完成后的伤害值；扣血最低降至 0，因此它可以大于实际失去的生命值。 |
| `type` | `const damage_type` | 伤害类型。 |
| `flags` | `const damage_flags` | 伤害标记。 |

## 响应

响应读取扣血后、伤害元素附着/反应后的最新 table，并安排后续效果。生命值为 0 时应在这些响应之后安排击倒结算。

`deal_damage` 的全灭终局检查使用响应后的当前生命值：一方角色生命值全部为 0 时判另一方获胜，双方全部为 0 时判双方失败。
