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

借用指定切换候选的只读费用引用。尚未计算时为默认费用，已经计算后为当前结果。对应角色的 ID 也可通过 [`switch_target`](switch_target.md) 查询。

## 注意

本操作只读取费用。通过 [`calculate_switch_cost`](calculate_switch_cost.md) 重新计算该角色的费用后，引用所见的结果随之更新；引用不保存独立快照。下一次推进或重建现场后，先前取得的引用失效。

候选计算抛出异常后，对应费用可能只更新了一部分；须完整重算成功后才能检查或采用。
