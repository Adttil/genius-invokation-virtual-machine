[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **switch_active_character**

# givm::execution_view<execution_state::action_selection>::switch_active_character

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void switch_active_character(
    std::size_t target_index, const dice_counts& paid_dice
) const noexcept;

void switch_active_character(
    const definition_library& library,
    const table& card_table,
    std::size_t target_index,
    const dice_counts& paid_dice
) const;
```
[`dice_counts`](../../../enums/dice_counts.md)
[`definition_library`](../../definition_library.md)
[`table`](../../../table/table.md)

选择要切换至的角色和支付的骰子。可采用该角色已经计算的切换费用，也可在选择时同步重新报价。

## 参数

| | |
| --- | --- |
| `target_index` | 从零开始的切换候选索引，须小于 [`switch_target_count()`](switch_target_count.md)。 |
| `paid_dice` | 本次实际支付的各类骰子数量，须满足采用的费用及持有数量。 |
| `library` | 与当前执行现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |

## 返回值

（无）

## 异常

带 `library` 和 `card_table` 的重载会传递费用响应抛出的异常。报价失败时不提交本次选择；该角色可能留下部分报价，须重新完整计算成功后再检查或采用。其他角色不受影响。

## 注意

只接收 `target_index` 和 `paid_dice` 的重载采用已计算费用。调用方须先通过 [`calculate_switch_cost`](calculate_switch_cost.md) 为该候选完整报价，并自行保证该报价可用；本操作不重新计算费用。

带 `library` 和 `card_table` 的重载先同步重新计算该角色的切换费用，成功后填写选择。本次调用完成报价与选择，无需在两者之间推进执行器。

两种重载都不自动检查支付是否合法。需要检查时，可先报价并调用 [`check_switch_payment`](check_switch_payment.md)，再用只接收 `target_index` 和 `paid_dice` 的重载采用该费用。

选择后，下一次 [`executor::step`](../../executor/step.md) 才执行已确认的费用效果、支付骰子和切换；不会再次报价。调用本函数不会修改牌桌或推进执行器。
