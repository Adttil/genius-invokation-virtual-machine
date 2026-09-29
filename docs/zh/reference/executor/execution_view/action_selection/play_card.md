[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **play_card**

# givm::execution_view<execution_state::action_selection>::play_card

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class TRandom>
execution_state play_card(
    const definition_library& library, table& card_table, TRandom& random,
    std::size_t card_index, const dice_counts& paid_dice, std::span<const card_target_id> targets = {}
) const;

template<class TRandom>
execution_state play_card_with_cached_cost(
    const definition_library& library, table& card_table, TRandom& random,
    std::size_t card_index, const dice_counts& paid_dice, std::span<const card_target_id> targets = {}
) const;
```

选择要打出的手牌、牌的目标与支付骰子。提交后立即推进，返回下一处暂停现场。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 非 `const`、非 `volatile` 的可调用对象类型，其无参数调用结果可隐式转换为 `std::uint32_t`。 |

## 参数

| | |
| --- | --- |
| `card_index` | 从零开始的出牌候选索引，须小于 [`card_count()`](card_count.md)。 |
| `paid_dice` | 本次支付的骰子，须满足采用的费用及持有数量。 |
| `targets` | 按选择顺序提供的目标 ID，默认空 span 表示不选目标。只采用前两个元素，多余元素忽略；缺少的位置补为 `std::monostate`。 |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `random` | 本次推进使用的随机源，以左值传入。 |

## 返回值

推进后到达的 [`execution_state`](../../execution_state.md)，可能是下一处输入、观察或终局现场。

## 异常

未定义 `NDEBUG` 时，现场已失效或种类错误会抛出 [`execution_view_error`](../../execution_view_error.md)；输入合法性结果失败时抛出 [`view_input_error`](../../view_input_error.md)；实体 ID 越界或已移除的诊断沿用 [`command_input_error`](../../command_input_error.md)。这些检查均在填写选择和开始推进之前。进入执行后，定义源、随机源及命令检查产生的异常向外传播，执行不提供回滚。

计算费用时的响应异常也向外传播。报价失败后，该候选在本现场不能重新报价或使用缓存。

## 注意

若即时报价已经成功，而后续 Debug 输入检查失败，报价仍然保留；修正输入后应调用 `play_card_with_cached_cost`，不能重新报价。

同一行动窗口内，每个候选只允许计算一次费用。`play_card` 同步计算报价后提交，只用于尚未报价的候选；`play_card_with_cached_cost` 使用已经完整计算的报价与对应支付效果，不重新计算。Debug 检查报价状态，Release 由调用方保证。

两种提交方式均在 Debug 下检查支付、目标及使用条件；目标按选择前缀依次验证，最终选择须允许完成。Release 不执行这些检查。调用方可以独立使用 [`card_payment_validate`](card_payment_validate.md) 与分步的 [`card_targets_validate`](card_targets_validate.md)，并负责保证输入合法、当前选择允许完成。

充能按费用要求自动从出战角色扣除，不需要另行选择支付量。目标由牌定义解释，未使用的位置忽略。无需目标时可直接调用 `play_card(library, card_table, random, card_index, paid_dice)`。本操作复制采用的目标 ID，调用完成后无需保留传入的目标范围。

本操作提交并推进，先让牌离开手牌，再执行确认的费用效果、扣除骰子与充能，依次处理骰子移除和充能变化通知，随后广播 [`card_will_be_played`](../../../definition/events/card_will_be_played.md)。若未被反制，则执行该牌的 [`card_effect`](../../../definition/events/card_effect.md)；之后均广播 [`card_played`](../../../definition/events/card_played.md)。反制只取消牌的原效果，不退还支付，也不撤销牌离手。

支援牌的替换目标由卡牌定义通过现有目标检查与效果表达：支援区已满时要求选择一个己方有效 `support_id`，效果先 [`remove_support`](../../../definition/commands/remove_support.md) 再 [`add_support`](../../../definition/commands/add_support.md)。行动视图不自动选择或移除支援；旧支援的离场响应完整结束后，添加命令重新依据当时容量决定是否生效。

这次行动沿用报价确定的行动速度：快速行动保留行动权，战斗行动按行动阶段规则交接。整个流程由 [`begin_action`](../../../definition/commands/begin_action.md) 处理。
