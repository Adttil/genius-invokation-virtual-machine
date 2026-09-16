[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **check_card_targets**

# givm::execution_view<execution_state::action_selection>::check_card_targets

定义于头文件 `<givm/executor.hpp>`

```cpp
card_target_check_result check_card_targets(
    const definition_library& library,
    const table& card_table,
    std::size_t card_index,
    std::span<const card_target_id> targets = {}
) const;
```
[`card_target_check_result`](../../../definition/events/card_target_check_result.md)
[`card_target_id`](../../../definition/events/card_target_id.md)

请牌的定义检查当前已选目标与用牌条件，并告知能否完成或继续选择。

## 参数

| | |
| --- | --- |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `card_index` | 从零开始的出牌候选索引，须小于 [`card_count()`](card_count.md)。 |
| `targets` | 按选择顺序提供的目标 ID，默认空 span 表示尚未选择目标。只采用前两个元素，多余元素忽略。 |

## 返回值

该牌对 [`card_target_check`](../../../definition/events/card_target_check.md) 给出的结果：

| | |
| --- | --- |
| `invalid` | 本步目标或用牌条件不合法。 |
| `valid_incomplete` | 当前选择有效，但尚不能出牌，必须继续选择。 |
| `valid_complete_or_continue` | 当前选择有效，可以出牌，也可以继续选择。 |
| `valid_complete` | 当前选择有效，可以出牌，不能继续选择。 |

没有对应响应时返回 `valid_complete`，适用于无需目标检查的牌。

## 异常

牌定义的检查响应抛出的异常会传递给调用方。

## 注意

响应事件仍含两个固定目标位置，未提供的位置补为 `std::monostate`，`target_count` 为采用的元素数量：`0` 检查空选择，`1` 检查第一目标，`2` 检查第二目标。省略 `targets` 即可询问是否允许不选目标、是否必须或可以继续选择。达到两个目标的上限后，响应应返回 `invalid` 或 `valid_complete`。

检查第二目标时，由调用方保证第一目标合法，响应可直接据此检查第二目标。检查不要求调用方曾经调用前一步，执行器不会补做前一步检查；修改第一目标或相关对局条件后，不能继续沿用原有的合法性结论。牌定义仍可拒绝不允许的目标数量。

本操作只调用本牌的检查响应，不验证支付，也不计算费用、提交行动或修改牌桌。响应不得使用随机数，调用随机函数属于未定义行为；响应仅修改检查结果并返回空入口。库不记录检查进度，直接按本次 `targets` 的长度确定检查步骤。

本操作与 [`check_card_payment`](check_card_payment.md) 独立，且不要求先报价。调用方仍须保证传入当前现场内有效的候选索引，并在选择出牌时保证目标与用牌条件成立；[`play_card`](play_card.md) 不会自动检查。
