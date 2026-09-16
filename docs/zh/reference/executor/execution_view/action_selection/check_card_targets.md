[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **check_card_targets**

# givm::execution_view<execution_state::action_selection>::check_card_targets

定义于头文件 `<givm/executor.hpp>`

```cpp
card_target_check_result check_card_targets(
    const definition_library& library,
    const table& card_table,
    hand_card_id card,
    const std::array<card_target_id, 2>& targets
) const;
```
[`card_target_check_result`](../../../definition/events/card_target_check_result.md)
[`card_target_id`](../../../definition/events/card_target_id.md)

请牌的定义检查本次目标与其他用牌条件。

## 参数

| | |
| --- | --- |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `card` | 当前行动玩家仍在手中的有效手牌 ID。 |
| `targets` | 固定两个目标位置；各位置的含义由该牌定义，未使用的位置忽略。 |

## 返回值

该牌对 [`card_target_check`](../../../definition/events/card_target_check.md) 给出的结果：`valid`、`invalid_first_target`、`invalid_second_target` 或 `unmet_condition`。牌定义决定检查顺序及返回结果；没有对应响应时返回 `valid`。

## 异常

牌定义的检查响应抛出的异常会传递给调用方。

## 注意

本操作只调用本牌的检查响应，不检查目标数量，不验证支付，也不计算费用、提交行动或修改牌桌。响应不得使用随机数，调用随机函数属于未定义行为；响应仅修改检查结果并返回空入口。

本操作与 [`check_card_payment`](check_card_payment.md) 独立，且不要求先报价。调用方仍须保证传入有效手牌，并在选择出牌时保证目标与用牌条件成立；[`play_card`](play_card.md) 不会自动检查。

