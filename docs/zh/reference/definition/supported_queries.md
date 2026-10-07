[givm](../../reference.md) / [定义](../definition.md) / **supported_queries**

# givm::supported_queries

定义于头文件 `<givm/definition.hpp>`

```cpp
template<definition_category TCategory>
struct supported_queries;
```

一种定义类别支持的查询集合。定义源可为其中的查询提供静态 `query`，未提供时使用相应的默认方法。

## 模板参数

| | |
| --- | --- |
| `TCategory` | [`definition_category`](../enums/definition_category.md) 中的定义类别。 |

## 支持的查询

| 定义类别 | 查询 |
| --- | --- |
| `definition_category::character` | [`character_initial_state`](queries/character_initial_state.md)、[`character_initial_skill`](queries/character_initial_skill.md)、[`character_reaction_override`](queries/character_reaction_override.md) |
| `definition_category::card` | [`card_initial_state`](queries/card_initial_state.md)、[`card_target_validation`](queries/card_target_validation.md)、[`card_equipment_target_validation`](queries/card_equipment_target_validation.md) |
| `definition_category::skill` | [`skill_initial_cost`](queries/skill_initial_cost.md)、[`skill_target_validation`](queries/skill_target_validation.md) |
| `definition_category::card_status` | [`card_state_modification`](queries/card_state_modification.md) |
| `definition_category::support` | [`support_state_limit`](queries/support_state_limit.md) |
| `definition_category::summon` | [`summon_state_limit`](queries/summon_state_limit.md) |
| `definition_category::combat_status` | [`combat_status_state_limit`](queries/combat_status_state_limit.md) |
| `definition_category::attachment` | [`attachment_state_limit`](queries/attachment_state_limit.md)、[`technique_initial_cost`](queries/technique_initial_cost.md)、[`technique_target_validation`](queries/technique_target_validation.md) |
| `definition_category::reaction` | [`reaction_aura`](queries/reaction_aura.md) |
| 其他定义类别 | 空列表。 |

列表提供 [`type_list`](../utils/type_list.md) 的操作。查询按定义类别组织，不按手牌、牌堆等实体形态另设列表；查询所需实体由其参数类型表达。
