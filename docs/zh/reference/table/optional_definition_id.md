[givm](../../reference.md) / [牌桌](../table.md) / **optional_definition_id**

# givm::optional_definition_id

```cpp
template<definition_category Category>
using optional_definition_id = variant_definition_id<definition_category::null, Category>;

using optional_card_definition_id = optional_definition_id<definition_category::card>;
using optional_card_status_definition_id = optional_definition_id<definition_category::card_status>;
using optional_support_definition_id = optional_definition_id<definition_category::support>;
using optional_summon_definition_id = optional_definition_id<definition_category::summon>;
using optional_combat_status_definition_id = optional_definition_id<definition_category::combat_status>;
using optional_character_definition_id = optional_definition_id<definition_category::character>;
using optional_skill_definition_id = optional_definition_id<definition_category::skill>;
using optional_attachment_definition_id = optional_definition_id<definition_category::attachment>;
using optional_history_summary_definition_id = optional_definition_id<definition_category::history_summary>;
using optional_reaction_definition_id = optional_definition_id<definition_category::reaction>;
```

可空的单类别定义身份。默认构造为空，赋值为 `nullptr` 可清空；判断非空后用 `get()` 或 `operator*()` 提取相应强类型 ID。

完整成员见 [`variant_definition_id`](variant_definition_id.md)。
