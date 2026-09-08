[返回](../events.md)

# healing

治疗实际加血前的可修改工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `const std::variant<hand_card_id, deck_card_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 治疗来源。 |
| `target` | `const character_id` | 接受治疗的角色。 |
| `value` | `std::uint32_t` | 治疗量，可修改。 |

## 响应

治疗加成、禁止治疗等效果按顺序直接修改 `value`；实体消耗由 handler 返回的固定响应程序完成。
