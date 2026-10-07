[givm](../../reference.md) / [牌桌](../table.md) / **optional_entity_id**

# givm::optional_entity_id

```cpp
template<entity_category Category>
using optional_entity_id = variant_entity_id<entity_category::null, Category>;

using optional_player_id = optional_entity_id<entity_category::player>;
using optional_hand_card_id = optional_entity_id<entity_category::hand_card>;
using optional_deck_card_id = optional_entity_id<entity_category::deck_card>;
using optional_hand_card_status_id = optional_entity_id<entity_category::hand_card_status>;
using optional_deck_card_status_id = optional_entity_id<entity_category::deck_card_status>;
using optional_support_id = optional_entity_id<entity_category::support>;
using optional_summon_id = optional_entity_id<entity_category::summon>;
using optional_combat_status_id = optional_entity_id<entity_category::combat_status>;
using optional_character_id = optional_entity_id<entity_category::character>;
using optional_skill_id = optional_entity_id<entity_category::skill>;
using optional_attachment_id = optional_entity_id<entity_category::attachment>;
using optional_reaction_id = optional_entity_id<entity_category::reaction>;
```

可空的单类别实体身份，例如尚未选择的出战角色或没有发生的反应。默认构造为空，判断非空后用 `get()` 或 `operator*()` 提取强类型 ID。

空身份与实体删除、角色击倒是不同的概念；完整成员见 [`variant_entity_id`](variant_entity_id.md)。
