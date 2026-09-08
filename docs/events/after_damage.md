[返回](../events.md)

# after_damage

角色生命值已经扣除，且该伤害携带的元素附着/反应流程已经处理后的时机。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `const std::variant<hand_card_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 伤害来源。 |
| `target` | `const character_id` | 受伤角色。 |
| `value` | `const std::uint32_t` | 本次扣血命令携带的值。 |
| `type` | `const damage_type` | 伤害类型。 |
| `flags` | `const damage_flags` | 伤害标记。 |

## 响应

响应读取扣血后、伤害元素附着/反应后的最新 table，并安排后续效果。生命值为 0 时应在这些响应之后安排击倒结算；当前尚未实现该后续流程。
