[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **card_initial_cost**

# givm::card_initial_cost

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_initial_cost
{
    using result_t = action_cost_requirement;
};
```

取得卡牌自身的骰子、充能费用与行动速度。出牌费用预览以此为基础，再通过 [`cost_of_card`](../events/cost_of_card.md) 响应处理本场对局中的增减费与速度调整。

## 成员类型

| | |
| --- | --- |
| `result_t` | [`action_cost_requirement`](../events/action_cost_requirement.md) |

## 注意

本查询没有参数，结果在编译定义库时按具体定义保存；每次报价直接读取，不重新调用定义源的查询函数，也不依赖所选目标。

未提供源查询时，[默认方法](../query_default.md)返回零骰子、零充能费用与 `action_speed::fast`。
