[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **check_card_payment**

# givm::execution_view<execution_state::action_selection>::check_card_payment

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr card_payment_check_result check_card_payment(
    const table& card_table,
    std::size_t card_index,
    const dice_counts& paid_dice
) const noexcept;
```
[`card_payment_check_result`](../../card_payment_check_result.md)

检查所选骰子能否支付指定手牌的出牌费用。

## 参数

| | |
| --- | --- |
| `card_table` | 当前行动发生的牌桌。 |
| `card_index` | 从零开始的出牌候选索引，须小于 [`card_count()`](card_count.md)，且该候选已经完整报价。 |
| `paid_dice` | 准备支付的各类骰子数量。 |

## 返回值

先判断所选骰子能否恰好满足费用，不匹配时立即返回 `requirement_mismatch`；匹配后检查当前行动玩家的持有数量，不足时返回 `insufficient_dice`；全部通过时返回 `valid`。

## 注意

费用匹配规则见 [`elemental_dice_requirement`](../../../definition/events/elemental_dice_requirement.md)。本操作读取已计算费用和牌桌，不重新报价、不检查目标、不提交行动或修改牌桌。

支付检查与 [`check_card_targets`](check_card_targets.md) 相互独立，由调用方按需使用；[`play_card`](play_card.md) 不会自动调用它们。即使检查通过，也须显式提供行动输入后才能继续推进。
