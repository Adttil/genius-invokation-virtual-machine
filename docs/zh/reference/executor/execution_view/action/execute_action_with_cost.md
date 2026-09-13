[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action>](../action.md) / **execute_action_with_cost**

# givm::execution_view<execution_state::action>::execute_action_with_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void execute_action_with_cost(stack_count_t index, const action_argument& argument) const noexcept;
```
[`stack_count_t`](../../../utils/stack_count_t.md)
[`action_argument`](../../instructions/action_argument.md)

提交指定候选的行动，采用该候选已经计算的费用。

## 参数

| | |
| --- | --- |
| `index` | 具有可用预览费用的切换候选下标。 |
| `argument` | 本次支付参数，调用方须保证满足采用的费用。 |

## 返回值

(无)

## 注意

本操作不检查候选是否已经报价，不重新报价；下一次推进才执行提交。
