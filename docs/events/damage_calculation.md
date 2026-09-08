[返回](../events.md)

# damage_calculation

伤害计算阶段的可修改工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `std::variant<hand_card_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 伤害来源，可修改。 |
| `target` | `character_id` | 目标角色，可修改。 |
| `value` | `std::uint32_t` | 加算伤害值，可修改。 |
| `multiplier_numerator` | `std::uint16_t` | 最终倍率分子，可修改。 |
| `multiplier_denominator` | `std::uint16_t` | 最终倍率分母，可修改。 |
| `type` | `damage_type` | 伤害类型，可被附魔修改。 |
| `flags` | `damage_flags` | 伤害标记，可修改。 |
| `already_handled_reaction` | `bool` | 是否已由定制效果处理伤害侧反应。 |

## 响应

增伤、倍率、附魔和元素幻变按响应顺序直接修改同一对象。此阶段尚未应用最终倍率。
