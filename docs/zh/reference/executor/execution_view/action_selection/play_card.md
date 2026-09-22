[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **play_card**

# givm::execution_view<execution_state::action_selection>::play_card

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void play_card(
    std::size_t card_index,
    const dice_counts& paid_dice,
    std::span<const card_target_id> targets = {}
) const noexcept;

void play_card(
    const definition_library& library,
    const table& card_table,
    std::size_t card_index,
    const dice_counts& paid_dice,
    std::span<const card_target_id> targets = {}
) const;
```

选择要打出的手牌、牌的目标与支付骰子。下一次推进才开始结算这次出牌。

## 参数

| | |
| --- | --- |
| `card_index` | 从零开始的出牌候选索引，须小于 [`card_count()`](card_count.md)。 |
| `paid_dice` | 本次支付的骰子，须满足采用的费用及持有数量。 |
| `targets` | 按选择顺序提供的目标 ID，默认空 span 表示不选目标。只采用前两个元素，多余元素忽略；缺少的位置补为 `std::monostate`。 |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |

## 返回值

（无）

## 异常

带定义库与牌桌的重载会传递费用响应抛出的异常。报价失败时不提交本次选择；该候选在当前行动窗口不能读取、检查、采用或重新计算。

## 注意

同一行动窗口内，每个候选只允许计算一次费用。带定义库与牌桌的重载只可用于尚未报价的候选；已报价的候选必须使用采用已计算费用的重载。调用方自行保证，库不进行运行期检查。

不带定义库与牌桌的重载采用已完整计算的费用。另一重载同步计算报价后填写选择；两者都不自动检查支付、目标或其他用牌条件。调用方可以独立使用 [`card_payment_validate`](card_payment_validate.md) 与分步的 [`card_targets_validate`](card_targets_validate.md)，并负责保证输入合法、当前选择允许完成。

充能按费用要求自动从出战角色扣除，不需要另行选择支付量。目标由牌定义解释，未使用的位置忽略。无需目标时可直接调用 `play_card(card_index, paid_dice)`。本操作复制采用的目标 ID，调用完成后无需保留传入的目标范围。

本操作不推进执行器，也不修改牌桌。下一次 [`step`](../../executor/step.md) 先让牌离开手牌，再执行确认的费用效果、扣除骰子与充能，依次处理骰子移除和充能变化通知，随后广播 [`card_will_be_played`](../../../definition/events/card_will_be_played.md)。若未被反制，则执行该牌的 [`card_effect`](../../../definition/events/card_effect.md)；之后均广播 [`card_played`](../../../definition/events/card_played.md)。反制只取消牌的原效果，不退还支付，也不撤销牌离手。

支援牌的替换目标由卡牌定义通过现有目标检查与效果表达：支援区已满时要求选择一个己方有效 `support_id`，效果先 [`remove_support`](../../../definition/commands/remove_support.md) 再 [`add_support`](../../../definition/commands/add_support.md)。行动视图不自动选择或移除支援；旧支援的离场响应完整结束后，添加命令重新依据当时容量决定是否生效。

这次行动沿用报价确定的行动速度：快速行动保留行动权，战斗行动按行动阶段规则交接。整个流程由 [`begin_action`](../../../definition/commands/begin_action.md) 处理。
