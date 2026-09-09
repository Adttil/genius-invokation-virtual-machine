[返回](../events.md)

# elemental_reaction_will_occur

元素反应默认效果被安排前的可接管工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `const std::variant<hand_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 附着来源。 |
| `target` | `const character_id` | 目标角色。 |
| `incoming_element` | `const element` | 新附着元素。 |
| `reacted_aura` | `const element_aura` | 反应前目标 aura。 |
| `reaction` | `const elemental_reaction` | 已判断出的原始反应类型。 |
| `cause` | `const element_application_cause` | 普通附着或伤害附着，默认 `element_application_cause::effect`。 |
| `already_handled` | `bool` | 是否由定制效果接管默认反应处理，默认 false，可修改。 |

## 响应

仅在已判断出反应时广播本事件；无反应的附着直接更新 aura，不触发本事件和 [`after_elemental_reaction`](after_elemental_reaction.md)。

元素幻变等效果若接管反应，应把 `already_handled` 设为 true，并返回执行全部替代效果的固定入口，其中包括需要的 aura 修改。全部响应完成后，领域指令仅在该标志仍为 false 时，按本事件记录的 `reacted_aura`、`incoming_element` 与 `reaction` 应用默认 aura 变化及反应效果。

由 `deal_damage` 触发时，本事件发生在扣血之后；伤害侧反应加成已经在 [`damage_calculation`](damage_calculation.md) 后处理，不能通过本事件追溯修改。
