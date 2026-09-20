[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **card_state_modification**

# givm::card_state_modification

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_state_modification
{
    using result_t = void;
    card_state& state;
    const status_state& status;
};
```

请卡牌附属状态的定义修改卡牌自身属性，例如提高这张牌的费用，或使其不能用于元素调和。同一查询适用于手牌和牌堆中的附属状态。

## 成员类型

| | |
| --- | --- |
| `result_t` | `void` |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `state` | [`card_state`](../../table/card_state.md)`&` | 要修改的卡牌属性 |
| `status` | `const` [`status_state`](../../table/status_state.md)`&` | 该附属状态自身的数据，只读 |

## 注意

查询由 `status_definition` 提供，定义的编译结果另由 `query` 的第一个参数传入。未提供查询时，[默认方法](../query_default.md)不修改卡牌属性。查询只修改 `state`，不能调用随机数或产生其他副作用。

维护卡牌属性时，新增附属状态只应用新增状态的查询；移除某个状态或改变影响属性的状态数据后，应恢复 [`card_initial_state`](card_initial_state.md)，再按附属状态顺序依次应用剩余有效状态。查询不要求幂等，也不需要实现逆操作。整张牌离场不属于这种属性重建，须保留它最后的属性。

本查询只在显式调用时生效；当前尚未提供卡牌附属状态的公开增删、修改命令。后续相应操作须遵守上述维护约定。
