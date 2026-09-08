[返回](../events.md)

# entity_left

实体已经被标记无效后的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `entity` | `const std::variant<hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 已离场实体的历史 ID。 |

## 响应

目标 ID 不可再解引用。其他实体可响应这次离场并返回固定入口执行后续效果。
