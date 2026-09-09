[返回](../events.md)

# healed

生命值已经恢复后的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `const std::variant<hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 治疗来源。 |
| `target` | `const character_id` | 接受治疗的角色。 |
| `value` | `const std::uint32_t` | 实际恢复量。 |

## 响应

响应读取治疗后的状态并安排“受到治疗后”效果。
