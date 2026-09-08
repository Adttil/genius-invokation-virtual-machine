[返回](../events.md)

# after_elemental_reaction

元素反应效果均已处理后的时机。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `const std::variant<hand_card_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 附着来源。 |
| `target` | `const character_id` | 目标角色。 |
| `incoming_element` | `const element` | 新附着元素。 |
| `reacted_aura` | `const element_aura` | 参与反应的旧 aura。 |
| `reaction` | `const elemental_reaction` | 发生的反应类型。 |
| `cause` | `const element_application_cause` | 普通附着或伤害附着。 |

## 响应

用于“触发某反应后”效果。默认效果或接管反应的固定响应程序已经完成；响应者在本时机开始时根据最新 table 枚举，因此反应过程中创建的新实体也可以响应。若反应由伤害携带的元素附着触发，本时机先于 [`after_damage`](after_damage.md)。
