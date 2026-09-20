[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **switch_cost**

# givm::execution_view<execution_state::action_selection>::switch_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr const cost_of_switch& switch_cost(std::size_t target_index) const noexcept;
```
[`cost_of_switch`](../../../definition/events/cost_of_switch.md)

取得指定角色的当前切换费用。

## 参数

| | |
| --- | --- |
| `target_index` | 从零开始的切换候选索引，须小于 [`switch_target_count()`](switch_target_count.md)。 |

## 返回值

指定候选已经完整计算的只读费用引用。

## 注意

调用方须先为该候选完成一次 [`calculate_switch_cost`](calculate_switch_cost.md)。本操作可反复调用，只读取已计算结果，不触发费用响应。

报价其他候选可能使之前取得的费用引用失效；下一次推进或重建现场也会使引用失效。需要再次读取时，通过本 view 重新取得引用。报价失败的候选不能读取、检查或采用。
