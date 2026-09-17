[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **skill_cost**

# givm::execution_view<execution_state::action_selection>::skill_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr const cost_of_skill& skill_cost(std::size_t skill_index) const noexcept;
```
[`cost_of_skill`](../../../definition/events/cost_of_skill.md)

取得指定技能的当前技能使用费用。

## 参数

| | |
| --- | --- |
| `skill_index` | 从零开始的技能候选索引，须小于 [`skill_count()`](skill_count.md)。 |

## 返回值

借用指定技能候选的只读费用引用。对应技能的 ID 也可通过 [`skill_id`](skill_id.md) 查询。

每项技能的费用须通过 [`calculate_skill_cost`](calculate_skill_cost.md) 完整计算后，才能检查支付或采用。

## 注意

本操作只读取费用，不计算费用、不选择技能使用。重新计算该技能后，引用所见的结果随之更新；引用不保存独立快照。下一次推进或重建现场后，先前取得的引用失效。

报价抛出异常后，对应费用可能只更新了一部分；须完整重算成功后才能检查或采用。
