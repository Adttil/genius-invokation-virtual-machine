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

费用响应抛出的异常会传递给调用方。失败后该候选可能只留下部分结果，不能读取、检查或采用，也不能在当前行动窗口重新计算。

## 注意

同一行动窗口内，每个候选只允许计算一次；调用方自行保证，库不进行运行期检查，重复计算属于未定义行为。计算完成后可反复调用 `card_cost` 读取缓存结果。

报价先复制该牌当前 [`card_state::cost`](../../../table/card_state.md) 作为基础费用，再处理 [`cost_of_card`](../../../definition/events/cost_of_card.md) 费用响应。报价不会写回卡牌自身的费用；卡牌初始费用默认是零骰子、零充能的快速行动。

报价无需先选择目标，目标及其他用牌条件通过 [`card_targets_validate`](card_targets_validate.md) 独立检查。可打出的牌提供原效果响应。

费用响应不得使用随机数，调用随机函数属于未定义行为。本操作无需随机源，同步完成，不选择出牌、不执行费用响应提交的后续效果，也不修改牌桌或推进执行器。

报价其他候选可能使之前取得的费用引用失效，下一次推进或重建现场也会使引用失效。需要再次读取时，通过本 view 的费用读取接口重新取得引用。完整报价后可独立检查 [支付](card_payment_validate.md) 与 [目标及用牌条件](card_targets_validate.md)，再通过 [`play_card`](play_card.md) 选择出牌。
