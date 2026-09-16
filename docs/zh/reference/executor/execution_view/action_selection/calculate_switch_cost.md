[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **calculate_switch_cost**

# givm::execution_view<execution_state::action_selection>::calculate_switch_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
const cost_of_switch& calculate_switch_cost(
    const definition_library& library,
    const table& card_table,
    std::size_t target_index
) const;
```
[`cost_of_switch`](../../../definition/events/cost_of_switch.md)
[`definition_library`](../../definition_library.md)
[`table`](../../../table/table.md)

计算切换至指定角色的费用，并立即返回计算结果。

## 参数

| | |
| --- | --- |
| `library` | 与当前执行现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `target_index` | 从零开始的切换候选索引，须小于 [`switch_target_count()`](switch_target_count.md)。 |

## 返回值

借用当前现场的只读费用引用，与 [`switch_cost(target_index)`](switch_cost.md) 读取的费用相同。

## 异常

费用响应抛出的异常会传递给调用方。失败后该候选的费用及提交效果可能只更新了一部分，不能采用；须重新完整计算成功后再检查或提交该候选。其他候选不受影响。

## 注意

每次调用都从该角色的默认切换费用重新计算，并更新确认行动后才执行的效果；其他角色的结果保持不变。本操作同步完成，不提交行动、不执行费用响应返回的后续效果，也不修改牌桌或推进执行器。

费用响应不得使用随机数；在费用响应中调用随机函数属于未定义行为。费用预览无需传入随机源。

返回的引用不是费用快照；再次计算同一候选会更新引用所见的结果。下一次推进或重建现场后，先前取得的引用失效。

计算完成后，可通过 [`check_switch_payment`](check_switch_payment.md) 检查所选骰子是否满足费用及持有数量，再通过 [`switch_active_character`](switch_active_character.md) 选择采用该费用的切换。
