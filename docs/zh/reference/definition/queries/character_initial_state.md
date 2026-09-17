[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **character_initial_state**

# givm::character_initial_state

定义于头文件 `<givm/definition.hpp>`

```cpp
struct character_initial_state
{
    using result_t = character_state;
};
```

取得角色定义的初始状态，包括最大生命、初始生命、充能上限和充能类型等。[`load_deck`](../../executor/load_deck.md) 与 [`enter_character`](../commands/enter_character.md) 使用该结果初始化角色。

## 成员类型

| | |
| --- | --- |
| `result_t` | [`character_state`](../../table/character_state.md) |

## 注意

本查询没有参数，结果在编译定义库时按具体定义保存；初始化角色时不会重新调用定义源的查询函数。未提供源查询时，[默认方法](../query_default.md)返回 `character_state{}`。

角色初始状态只依赖编译后的定义，不依赖入场玩家、其他实体或当前牌桌。需要在入场之后产生的其他效果应另行表达。

使用替代充能的定义可通过 `tag_dependencies()` 声明资源标签，在 `compile` 中用 [`resolve_tag`](../../executor/definition_compile_context/resolve_tag.md) 取得 ID，再将其写入本查询结果的 `energy_tag`；加载牌组时直接采用该初始状态，无需执行被动技能响应。
