[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **execute_action_with_cost**

# givm::execution_view<execution_state::action_selection>::execute_action_with_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void execute_action_with_cost(stack_count_t index, const action_argument& argument) const noexcept;
```
[`stack_count_t`](../../../utils/stack_count_t.md)
[`action_argument`](../../action_argument.md)

提交指定候选的行动，采用该候选已经计算的费用。

## 参数

| | |
| --- | --- |
| `index` | 具有可用预览费用的切换候选下标。 |
| `argument` | 本次支付参数，调用方须保证满足采用的费用。 |

## 返回值

(无)

## 注意

先通过 [`calculate_cost`](calculate_cost.md) 完整计算该候选的费用，并保证支付符合费用及持有数量；可调用 [`check_payment`](check_payment.md) 检查。本操作不检查候选是否已经报价或支付是否合法，不重新报价；下一次推进才执行提交。
