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

费用响应抛出的异常会传递给调用方。失败后该候选可能只留下部分结果，不能读取、检查或采用，也不能在当前行动窗口重新计算。

## 注意

同一行动窗口内，每个候选只允许计算一次；调用方自行保证，库不进行运行期检查，重复计算属于未定义行为。计算完成后可反复调用 `switch_cost` 读取缓存结果。

从默认切换费用计算，并准备确认行动后才执行的效果；其他角色的结果保持不变。本操作同步完成，不提交行动、不执行费用响应提交的后续效果，也不修改牌桌或推进执行器。

费用响应不得使用随机数；在费用响应中调用随机函数属于未定义行为。费用预览无需传入随机源。

报价其他候选可能使之前取得的费用引用失效，下一次推进或重建现场也会使引用失效。需要再次读取时，通过本 view 的费用读取接口重新取得引用。

计算完成后，可通过 [`switch_payment_validate`](switch_payment_validate.md) 检查所选骰子是否满足费用及持有数量，再通过 [`switch_active_character`](switch_active_character.md) 选择采用该费用的切换。
