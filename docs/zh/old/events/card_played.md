[返回](../events.md)

# card_played

牌已离开手牌后的只读通知；字段会说明定义效果是否被无效化。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 原手牌历史 ID。 |
| `definition_id` | `const definition_id<card_definition>` | 牌定义。 |
| `target` | `const std::variant<std::monostate, character_id, support_id, summon_id>` | 最终目标。 |
| `speed` | `const action_speed` | 最终行动速度。 |
| `effect_cancelled` | `const bool` | 牌定义效果是否被无效化。 |

## 响应

“打出牌后”的固定响应程序位于牌自身效果之后。
