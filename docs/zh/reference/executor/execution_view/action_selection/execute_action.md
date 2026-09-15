[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **execute_action**

# givm::execution_view<execution_state::action_selection>::execute_action

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void execute_action(stack_count_t index, const action_argument& argument) const noexcept;
```
[`stack_count_t`](../../../utils/stack_count_t.md)
[`action_argument`](../../action_argument.md)

提交指定候选的行动，在执行时重新计算费用。

## 参数

| | |
| --- | --- |
| `index` | 有效的切换候选下标。 |
| `argument` | 本次支付参数，调用方须保证满足重新计算后的费用。 |

## 返回值

(无)

## 注意

本操作只填写输入。下一次推进连续完成报价与提交，不在两者之间再次等待输入。

提交时不自动检查支付是否合法。需要先确认费用及支付时，可通过 [`calculate_cost`](calculate_cost.md) 与 [`check_payment`](check_payment.md) 完成预览和检查，再使用 [`execute_action_with_cost`](execute_action_with_cost.md) 采用该费用。
