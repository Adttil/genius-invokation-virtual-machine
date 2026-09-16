[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **card_cost**

# givm::execution_view<execution_state::action_selection>::card_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr const cost_of_card& card_cost(std::size_t card_index) const noexcept;
```
[`cost_of_card`](../../../definition/events/cost_of_card.md)

取得指定手牌的当前出牌费用。

## 参数

| | |
| --- | --- |
| `card_index` | 从零开始的出牌候选索引，须小于 [`card_count()`](card_count.md)。 |

## 返回值

借用指定出牌候选的只读费用引用。对应手牌的 ID 也可通过 [`card_id`](card_id.md) 查询。

每张牌的费用须通过 [`calculate_card_cost`](calculate_card_cost.md) 完整计算后，才能检查支付或采用。

## 注意

本操作只读取费用，不计算费用、不选择出牌。重新计算该牌后，引用所见的结果随之更新；引用不保存独立快照。下一次推进或重建现场后，先前取得的引用失效。

报价抛出异常后，对应费用可能只更新了一部分；须完整重算成功后才能检查或采用。
