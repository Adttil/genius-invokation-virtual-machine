[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **calculate_skill_cost**

# givm::execution_view<execution_state::action_selection>::calculate_skill_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
const cost_of_skill& calculate_skill_cost(
    const definition_library& library, const table& card_table, std::size_t skill_index
) const;
```
[`cost_of_skill`](../../../definition/events/cost_of_skill.md)

计算使用指定技能的费用，并立即返回结果。

## 参数

| | |
| --- | --- |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `skill_index` | 从零开始的技能候选索引，须小于 [`skill_count()`](skill_count.md)。 |

## 返回值

借用当前现场的只读费用引用，与 [`skill_cost(skill_index)`](skill_cost.md) 读取的费用相同。

## 异常

费用响应抛出的异常会传递给调用方。失败后该候选可能只留下部分结果，不能读取、检查或采用，也不能在当前行动窗口重新计算。

## 注意

同一行动窗口内，每个候选只允许计算一次；调用方自行保证，库不进行运行期检查，重复计算属于未定义行为。计算完成后可反复调用 `skill_cost` 读取缓存结果。

报价先读取该技能定义已保存的 [`skill_initial_cost`](../../../definition/queries/skill_initial_cost.md) 结果作为基础费用，再处理 [`cost_of_skill`](../../../definition/events/cost_of_skill.md) 费用响应。未提供初始费用查询时，默认需求为零骰子、零充能的战斗行动。

报价无需先选择目标，目标及其他技能使用条件通过 [`skill_targets_validate`](skill_targets_validate.md) 独立检查。候选技能均支持主动效果响应。

费用响应不得使用随机数，调用随机函数属于未定义行为。本操作无需随机源，同步完成，不提交技能行动、不执行费用响应提交的后续效果，也不修改牌桌或推进执行器。

报价其他候选可能使之前取得的费用引用失效，下一次推进或重建现场也会使引用失效。需要再次读取时，通过本 view 的费用读取接口重新取得引用。完整报价后可独立检查 [支付](skill_payment_validate.md) 与 [目标及技能使用条件](skill_targets_validate.md)，再通过 [`use_skill`](use_skill.md) 选择使用技能。
