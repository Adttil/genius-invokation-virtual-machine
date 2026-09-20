[givm](../../reference.md) / [定义](../definition.md) / **supported_queries**

# givm::supported_queries

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TCategory>
struct supported_queries;
```

一种定义类别支持的查询集合。定义源可为其中的查询提供静态 `query`，未提供时使用相应的默认方法。

## 模板参数

| | |
| --- | --- |
| `TCategory` | [`definition_types`](definition_types.md) 中的定义类别。 |

## 支持的查询

| 定义类别 | 查询 |
| --- | --- |
| `character_view` | [`character_initial_state`](queries/character_initial_state.md)、[`character_initial_skill`](queries/character_initial_skill.md) |
| `card_definition` | [`card_initial_state`](queries/card_initial_state.md)、[`card_target_validation`](queries/card_target_validation.md) |
| `skill_view` | [`skill_initial_cost`](queries/skill_initial_cost.md)、[`skill_target_validation`](queries/skill_target_validation.md) |
| `status_definition` | [`card_state_modification`](queries/card_state_modification.md) |
| 其他定义类别 | 空列表。 |

列表提供 [`type_list`](../utils/type_list.md) 的操作。查询按定义类别组织，不按手牌、牌堆等实体形态另设列表；查询所需实体由其参数类型表达。
