[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **card_payment_validate**

# givm::execution_view<execution_state::action_selection>::card_payment_validate

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr card_payment_validation card_payment_validate(
    const table& card_table,
    std::size_t card_index,
    const dice_counts& paid_dice
) const noexcept;
```
[`card_payment_validation`](../../card_payment_validation.md)

检查所选骰子与出战角色充能能否支付指定手牌的出牌费用。

## 参数

| | |
| --- | --- |
| `card_table` | 当前行动发生的牌桌。 |
| `card_index` | 从零开始的出牌候选索引，须小于 [`card_count()`](card_count.md)，且该候选已经完整报价。 |
| `paid_dice` | 准备支付的各类骰子数量。 |

## 返回值

依次进行以下检查，遇到第一个失败立即返回：

1. 所选骰子能否恰好满足费用，不匹配时返回 `requirement_mismatch`。
2. 当前行动玩家是否持有所选骰子，不足时返回 `insufficient_dice`。
3. 充能费用非零时，出战角色与费用的 `energy_tag` 是否相等，不匹配时返回 `energy_tag_mismatch`。充能费用为零时忽略类型。
4. 出战角色充能数量是否足够，不足时返回 `insufficient_energy`。

全部通过时返回 `valid`。

## 注意

费用匹配规则见 [`elemental_dice_requirement`](../../../enums/elemental_dice_requirement.md)。本操作读取已计算费用和牌桌，不重新报价、不检查目标、不提交行动或修改牌桌。

支付检查与 [`card_targets_validate`](card_targets_validate.md) 相互独立，由调用方按需使用；[`play_card`](play_card.md) 不会自动调用它们。即使检查通过，也须显式提供行动输入后才能继续推进。
