[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **check_payment**

# givm::execution_view<execution_state::action_selection>::check_payment

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr payment_check_result check_payment(
    const table& card_table,
    stack_count_t index,
    const dice_counts& paid_dice
) const noexcept;
```
[`payment_check_result`](../../payment_check_result.md)
[`table`](../../../table/table.md)
[`stack_count_t`](../../../utils/stack_count_t.md)
[`dice_counts`](../../../enums/dice_counts.md)

检查为指定切换候选选择的骰子能否支付其费用。

## 参数

| | |
| --- | --- |
| `card_table` | 当前行动发生的牌桌。 |
| `index` | 已经完整计算费用的有效切换候选下标。 |
| `paid_dice` | 准备支付的各类骰子数量。 |

## 返回值

先按该候选的费用判断支付骰子的种类和总数，符合后再检查当前行动玩家是否持有这些骰子：

| | |
| --- | --- |
| `payment_check_result::requirement_mismatch` | 所选骰子不符合费用要求。 |
| `payment_check_result::insufficient_dice` | 所选骰子符合费用要求，但持有数量不足。 |
| `payment_check_result::valid` | 所选骰子符合费用要求，且持有数量足够。 |

## 注意

先通过 [`calculate_cost`](calculate_cost.md) 完整计算该候选的费用。本操作只读取已计算费用和牌桌，不计算费用、提交行动、执行费用响应的后续效果或修改牌桌；提交接口也不会自动调用它。

所选骰子须恰好支付 [`elemental_dice_requirement`](../../../definition/events/elemental_dice_requirement.md) 的 `fixed`、`same` 和 `any` 三部分，具体匹配规则见该类型。费用要求与持有数量均不满足时，返回 `requirement_mismatch`。

支付检查只涉及费用和所选骰子，不检查其他行动参数。当前切换目标由候选下标确定，没有额外的目标参数检查。
