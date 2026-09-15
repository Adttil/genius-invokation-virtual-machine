[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **calculate_cost**

# givm::execution_view<execution_state::action_selection>::calculate_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
const cost_of_switch& calculate_cost(
    const definition_library& library,
    const table& card_table,
    stack_count_t index,
    TRandom& random_source
) const;
```
[`cost_of_switch`](../../../definition/events/cost_of_switch.md)
[`definition_library`](../../definition_library.md)
[`table`](../../../table/table.md)
[`stack_count_t`](../../../utils/stack_count_t.md)

计算指定切换候选的费用，并立即返回计算结果。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 非 `const`、非 `volatile` 的对象类型；其左值须可无参数调用，且结果可隐式转换为 `std::uint32_t`。 |

## 参数

| | |
| --- | --- |
| `library` | 与当前执行现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `index` | 有效的切换候选下标。 |
| `random_source` | 本次计算使用的随机源，以左值传入并传给费用响应；返回后不再持有它。 |

## 返回值

借用当前现场的只读费用引用，与 [`costs()`](costs.md) 中对应候选的费用相同。

## 异常

费用响应抛出的异常会传递给调用方。失败后该候选的费用及提交效果可能只更新了一部分，不能采用；须重新完整计算成功后再检查或提交该候选。其他候选不受影响。

## 注意

每次调用都从该候选的默认费用重新计算，并更新确认行动后才执行的效果；其他候选的结果保持不变。本操作同步完成，无需推进执行器。

费用响应按约定不得使用随机数，使反复预览不会改变对局的随机序列。与 [`executor::step`](../../executor/step.md) 一样，本接口接收并传递调用方的随机源；这一约定不由接口强制检查。

返回的引用不是费用快照；再次计算同一候选会更新引用所见的结果。下一次推进或重建现场后，先前取得的引用失效。

计算完成后，可通过 [`check_payment`](check_payment.md) 检查所选骰子是否满足费用及持有数量，再提交采用该费用的行动。
