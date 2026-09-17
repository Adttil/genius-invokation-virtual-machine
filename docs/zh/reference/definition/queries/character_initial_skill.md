[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **character_initial_skill**

# givm::character_initial_skill

定义于头文件 `<givm/definition.hpp>`

```cpp
struct character_initial_skill
{
    using result_t = definition_id<skill_view>;
    std::size_t skill_index;
};
```

按索引取得角色初始具有的一项技能定义。[`load_deck`](../../executor/load_deck.md) 与 [`enter_character`](../commands/enter_character.md) 使用这些定义为角色建立技能集合。

## 成员类型

| | |
| --- | --- |
| `result_t` | `definition_id<skill_view>` |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `skill_index` | `std::size_t` | 从零开始的初始技能索引。 |

## 返回值

指定技能的定义 ID。超出初始技能范围时返回无效 ID；缺少源查询时，[默认方法](../query_default.md)始终返回无效 ID，表示没有初始技能。

## 注意

初始化从索引零开始逐项查询，首次返回无效 ID 即结束。有效结果须连续提供，并且属于当前定义库中的技能定义；相关依赖按[定义源协议](../source_protocol.md)声明和解析。

本查询含索引参数，在需要时调用定义源，不作为无参查询缓存。定义源如何保存或生成结果由自身决定，不要求使用任何特定容器。

初始集合可以包含主动技能与被动技能；它只决定创建时具有哪些技能，不限制角色之后的技能集合。
