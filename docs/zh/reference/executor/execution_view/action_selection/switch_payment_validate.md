[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **switch_payment_validate**

# givm::execution_view<execution_state::action_selection>::switch_payment_validate

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr switch_payment_validation switch_payment_validate(
    const table& card_table,
    std::size_t target_index,
    const dice_counts& paid_dice
) const noexcept;
```
[`switch_payment_validation`](../../switch_payment_validation.md)
[`table`](../../../table/table.md)
[`dice_counts`](../../../enums/dice_counts.md)

检查所选骰子能否支付切换至指定角色的费用。

## 参数

| | |
| --- | --- |
| `card_table` | 当前行动发生的牌桌。 |
| `target_index` | 从零开始的切换候选索引，须小于 [`switch_target_count()`](switch_target_count.md)，且该候选已经完整报价。 |
| `paid_dice` | 准备支付的各类骰子数量。 |

## 返回值

先按该候选的费用判断支付骰子的种类和总数，符合后再检查当前行动玩家是否持有这些骰子：

| | |
| --- | --- |
| `switch_payment_validation::requirement_mismatch` | 所选骰子不符合费用要求。 |
| `switch_payment_validation::insufficient_dice` | 所选骰子符合费用要求，但持有数量不足。 |
| `switch_payment_validation::valid` | 所选骰子符合费用要求，且持有数量足够。 |

## 注意

先通过 [`calculate_switch_cost`](calculate_switch_cost.md) 完整计算该角色的切换费用，由调用方保证报价可用。本操作只读取已计算费用和牌桌，不计算费用、提交行动、执行费用响应的后续效果或修改牌桌；提交接口也不会自动调用它。

所选骰子须恰好支付 [`elemental_dice_requirement`](../../../definition/events/elemental_dice_requirement.md) 的 `fixed`、`same` 和 `any` 三部分，具体匹配规则见该类型。费用不匹配时立即返回 `requirement_mismatch`，匹配后才检查持有数量。

支付检查只涉及费用和所选骰子；传入当前现场内有效的候选索引仍是调用前提。
