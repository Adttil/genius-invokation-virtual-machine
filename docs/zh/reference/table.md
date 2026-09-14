[givm](../reference.md) / **牌桌**

# 牌桌

牌桌描述一场对局中双方拥有的角色、卡牌、骰子和持续效果，以及它们随对局变化的状态。

定义与实体、ID、只读视图和生命周期见[实体的身份与访问](table/entity_access.md)。

## 类

### 牌桌

|  |  |
| --- | --- |
| [`table`](table/table.md) | 一场游戏的牌桌 |

### 定义身份与牌组

|  |  |
| --- | --- |
| [`issued_id`](table/issued_id.md) | 按类别区分的已发行 ID |
| [`card_definition`](table/card_definition.md) | 卡牌定义类别 |
| [`status_definition`](table/status_definition.md) | 卡牌状态定义类别 |
| [`linked_deck`](table/linked_deck.md) | 已确定卡牌与角色定义的牌组 |

### 实体视图

|  |  |
| --- | --- |
| [`player_view`](table/player_view.md) | 玩家的只读视图 |
| [`hand_card_view`](table/hand_card_view.md) | 手牌的只读视图 |
| [`deck_card_view`](table/deck_card_view.md) | 牌库卡牌的只读视图 |
| [`hand_card_status_view`](table/hand_card_status_view.md) | 手牌上的状态的只读视图 |
| [`deck_card_status_view`](table/deck_card_status_view.md) | 牌库卡牌上的状态的只读视图 |
| [`support_view`](table/support_view.md) | 支援的只读视图 |
| [`summon_view`](table/summon_view.md) | 召唤物的只读视图 |
| [`combat_status_view`](table/combat_status_view.md) | 出战状态的只读视图 |
| [`character_view`](table/character_view.md) | 角色的只读视图 |
| [`skill_view`](table/skill_view.md) | 技能的只读视图 |
| [`attachment_view`](table/attachment_view.md) | 角色附属实体的只读视图 |

### 实体 ID

|  |  |
| --- | --- |
| [`player_id`](table/player_id.md) | 玩家的身份 |
| [`hand_card_id`](table/hand_card_id.md) | 手牌的身份 |
| [`deck_card_id`](table/deck_card_id.md) | 牌库卡牌的身份 |
| [`hand_card_status_id`](table/hand_card_status_id.md) | 手牌上的状态的身份 |
| [`deck_card_status_id`](table/deck_card_status_id.md) | 牌库卡牌上的状态的身份 |
| [`support_id`](table/support_id.md) | 支援的身份 |
| [`summon_id`](table/summon_id.md) | 召唤物的身份 |
| [`combat_status_id`](table/combat_status_id.md) | 出战状态的身份 |
| [`character_id`](table/character_id.md) | 角色的身份 |
| [`skill_id`](table/skill_id.md) | 技能的身份 |
| [`attachment_id`](table/attachment_id.md) | 角色附属实体的身份 |

### 状态与参数

|  |  |
| --- | --- |
| [`game_parameters`](table/game_parameters.md) | 对局采用的容量限制 |
| [`table_state`](table/table_state.md) | 双方共享的对局状态，用于记录当前回合和行动归属 |
| [`player_state`](table/player_state.md) | 一位玩家的骰子和出战角色状态 |
| [`character_state`](table/character_state.md) | 角色在对局中的生命值、充能和元素附着 |
| [`support_state`](table/support_state.md) | 支援在对局中使用的计数状态 |
| [`summon_state`](table/summon_state.md) | 召唤物在对局中使用的计数状态 |
| [`combat_status_state`](table/combat_status_state.md) | 出战状态在对局中使用的计数状态 |
| [`attachment_state`](table/attachment_state.md) | 角色附属实体在对局中使用的计数状态 |
| [`skill_state`](table/skill_state.md) | 技能在对局中使用的计数状态 |
| [`status_state`](table/status_state.md) | 卡牌状态在对局中使用的计数状态 |
| [`card_state`](table/card_state.md) | 卡牌自身的状态 |

## 类型别名

### 定义与标签 ID

|  |  |
| --- | --- |
| [`definition_id`](table/definition_id.md) | 实体采用的定义身份 |
| [`tag_id`](table/tag_id.md) | 分类标签的身份 |

## 函数

|  |  |
| --- | --- |
| [`other_player`](table/other_player.md) | 取得另一方玩家的 ID |
