[返回](../events.md)

# damage_calculation

伤害计算阶段的可修改工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `std::variant<hand_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 伤害来源，可修改。 |
| `target` | `character_id` | 目标角色，可修改。 |
| `value` | `std::uint32_t` | 加算伤害值，可修改。 |
| `multiplier_numerator` | `std::uint16_t` | 最终倍率分子，默认 1，可修改。 |
| `multiplier_denominator` | `std::uint16_t` | 最终倍率分母，默认 1，可修改，但不得为 0。 |
| `type` | `damage_type` | 伤害类型，可被附魔修改。 |
| `flags` | `damage_flags` | 伤害标记，可修改。 |
| `already_handled_reaction` | `bool` | 是否跳过核心的伤害侧默认反应加成，默认 false，可修改。 |

## 响应

增伤、倍率、附魔和元素幻变按响应顺序直接修改同一对象。此阶段尚未应用最终倍率。

全部响应完成后，若 `already_handled_reaction == false`，`deal_damage` 根据最终 `target` 的当时 aura 与最终 `type` 判断伤害侧默认反应加成，再统一应用倍率。`already_handled_reaction` 不会跳过随后扣血后的元素附着流程，也不会设置 [`elemental_reaction_will_occur`](elemental_reaction_will_occur.md) 的 `already_handled`。
