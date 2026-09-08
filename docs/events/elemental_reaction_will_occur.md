[返回](../events.md)

# elemental_reaction_will_occur

元素反应默认效果被安排前的可接管工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `const std::variant<hand_card_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 附着来源。 |
| `target` | `const character_id` | 目标角色。 |
| `incoming_element` | `const element` | 新附着元素。 |
| `reacted_aura` | `const element_aura` | 反应前目标 aura。 |
| `reaction` | `const elemental_reaction` | 已判断出的原始反应类型。 |
| `cause` | `const element_application_cause` | 普通附着或伤害附着。 |
| `already_handled` | `bool` | 是否已由定制效果完整处理。 |

## 响应

元素幻变等效果若接管反应，应把 `already_handled` 设为 true，并返回执行全部替代效果的固定入口；否则领域指令应用默认 aura 和反应效果。
