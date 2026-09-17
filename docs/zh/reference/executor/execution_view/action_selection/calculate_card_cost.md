[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **calculate_card_cost**

# givm::execution_view<execution_state::action_selection>::calculate_card_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
const cost_of_card& calculate_card_cost(
    const definition_library& library,
    const table& card_table,
    std::size_t card_index
) const;
```
[`cost_of_card`](../../../definition/events/cost_of_card.md)

计算打出指定手牌的费用，并立即返回结果。

## 参数

| | |
| --- | --- |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `card_index` | 从零开始的出牌候选索引，须小于 [`card_count()`](card_count.md)。 |

## 返回值

借用当前现场的只读费用引用，与 [`card_cost(card_index)`](card_cost.md) 读取的费用相同。

## 异常

响应抛出的异常会传递给调用方。失败后该牌的费用及确认后效果可能只更新了一部分，须重新完整计算成功后再检查支付或采用；其他候选不受影响。

## 注意

每次报价先读取该牌定义已保存的 [`card_initial_cost`](../../../definition/queries/card_initial_cost.md) 结果作为基础费用，再处理 [`cost_of_card`](../../../definition/events/cost_of_card.md) 费用响应。未提供初始费用查询时，默认需求为零骰子、零充能的快速行动。报价不沿用此前结果。

报价无需先选择目标，目标及其他用牌条件通过 [`card_targets_validate`](card_targets_validate.md) 独立检查。可打出的牌提供原效果响应。

费用响应不得使用随机数，调用随机函数属于未定义行为。本操作无需随机源，同步完成，不选择出牌、不执行费用响应返回的后续效果，也不修改牌桌或推进执行器。

返回引用不是快照；再次报价会更新其内容。下一次推进或重建现场后，先前引用失效。完整报价后可独立检查 [支付](card_payment_validate.md) 与 [目标及用牌条件](card_targets_validate.md)，再通过 [`play_card`](play_card.md) 选择出牌。
