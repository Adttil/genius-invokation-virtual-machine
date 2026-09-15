[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **costs**

# givm::execution_view<execution_state::action_selection>::costs

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr std::span<const cost_of_switch> costs() const noexcept;
```
[`cost_of_switch`](../../../definition/events/cost_of_switch.md)

取得当前切换候选的费用。

## 返回值

借用当前现场的只读费用范围，下标与行动候选一致。尚未计算的候选显示默认费用，已经计算的候选显示其当前结果。

## 注意

本操作只读取费用。通过 [`calculate_cost`](calculate_cost.md) 重新计算候选后，范围中的对应结果随之更新；该范围不保存独立快照。

候选计算抛出异常后，对应费用可能只更新了一部分；须完整重算成功后才能检查或采用。
