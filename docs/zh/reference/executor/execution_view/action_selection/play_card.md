[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **play_card**

# givm::execution_view<execution_state::action_selection>::play_card

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void play_card(
    hand_card_id card,
    const std::array<card_target_id, 2>& targets,
    const dice_counts& paid_dice
) const noexcept;

void play_card(
    const definition_library& library,
    const table& card_table,
    hand_card_id card,
    const std::array<card_target_id, 2>& targets,
    const dice_counts& paid_dice
) const;
```

选择要打出的手牌、牌的目标与支付骰子。下一次推进才开始结算这次出牌。

## 参数

| | |
| --- | --- |
| `card` | 当前行动玩家仍在手中的有效手牌 ID。 |
| `targets` | 两个固定目标位置，由牌定义解释；未使用的位置忽略。 |
| `paid_dice` | 本次支付的骰子，须满足采用的费用及持有数量。 |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |

## 返回值

（无）

## 异常

带定义库与牌桌的重载会传递报价响应的异常。报价失败时不提交本次选择；该牌须重新完整报价成功后才能检查支付或采用。

## 注意

不带定义库与牌桌的重载采用已完整计算的费用。另一重载同步重新报价后填写选择；两者都不自动检查支付、目标或其他用牌条件。调用方可以独立使用 [`check_card_payment`](check_card_payment.md) 与 [`check_card_targets`](check_card_targets.md)，并负责保证输入合法。

本操作不推进执行器，也不修改牌桌。下一次 [`step`](../../executor/step.md) 先让牌离开手牌，再执行确认的费用效果、扣除骰子并处理骰子变化响应，随后广播 [`card_will_be_played`](../../../definition/events/card_will_be_played.md)。若未被反制，则执行该牌的 [`card_effect`](../../../definition/events/card_effect.md)；之后均广播 [`card_played`](../../../definition/events/card_played.md)。反制只取消牌的原效果，不退还支付，也不撤销牌离手。

这次行动沿用报价确定的行动速度：快速行动保留行动权，战斗行动按行动阶段规则交接。整个流程由 [`begin_action`](../../../definition/commands/begin_action.md) 处理。

