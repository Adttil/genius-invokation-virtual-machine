[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **card_initial_state**

# givm::card_initial_state

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_initial_state
{
    using result_t = card_state;
};
```

取得一种卡牌的初始属性，包括自身费用、行动速度以及是否允许元素调和。

## 成员类型

| | |
| --- | --- |
| `result_t` | [`card_state`](../../table/card_state.md) |

## 注意

本查询没有参数，结果在编译定义库时按具体定义保存；初始化新牌时直接复制，不再调用定义源的查询函数。[`load_deck`](../../executor/load_deck.md)、[`insert_deck_card`](../commands/insert_deck_card.md) 与 [`create_hand_card`](../commands/create_hand_card.md) 都采用这个初始结果。区域转移保留原牌的状态，不重新初始化。

结果的 `cost.energy_tag` 指定费用需要的充能类型。需要替代充能时，定义源通过 `tag_dependencies()` 声明资源标签，在 `compile` 中用 [`resolve_tag`](../../executor/definition_compile_context/resolve_tag.md) 取得 ID，再写入本查询结果。

未提供源查询时，[默认方法](../query_default.md)返回 `card_state{}`：零骰子、零充能费用，行动速度为 `action_speed::fast`，允许元素调和。出牌报价使用牌当前的 `state().cost`，不直接使用本查询结果。
