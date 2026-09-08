[返回](../events.md)

# entity_will_leave

实体仍然有效时的离场前通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `entity` | `const std::variant<hand_card_id, deck_card_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 即将离场且仍可读取的实体。 |

## 响应

目标实体自身可以在这一阶段被定点通知并安排离场效果；响应仍可读取其定义和状态。
