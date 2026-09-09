[返回](../events.md)

# card_candidate_chosen

玩家已经从无状态候选列表选定一个牌定义。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `const player_id` | 作出选择的玩家。 |
| `definition_id` | `const definition_id<card_definition>` | 被选择的牌定义。 |

## 响应

响应可安排选择完成后的附加效果；候选项本身不是实体。
