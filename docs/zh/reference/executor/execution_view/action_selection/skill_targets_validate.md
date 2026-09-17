[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **skill_targets_validate**

# givm::execution_view<execution_state::action_selection>::skill_targets_validate

定义于头文件 `<givm/executor.hpp>`

```cpp
target_validation skill_targets_validate(
    const definition_library& library, const table& card_table,
    std::size_t skill_index, std::span<const skill_target_id> targets = {}
) const;
```
[`target_validation`](../../../definition/queries/target_validation.md)
[`skill_target_id`](../../../definition/events/skill_target_id.md)

请技能的定义检查当前已选目标与技能使用条件，并告知能否完成或继续选择。

## 参数

| | |
| --- | --- |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `skill_index` | 从零开始的技能候选索引，须小于 [`skill_count()`](skill_count.md)。 |
| `targets` | 按选择顺序提供的目标 ID，默认空 span 表示尚未选择目标。只采用前两个元素，多余元素忽略。 |

## 返回值

该技能对 [`skill_target_validation`](../../../definition/queries/skill_target_validation.md) 给出的结果：

| | |
| --- | --- |
| `invalid` | 本步目标或技能使用条件不合法。 |
| `valid_incomplete` | 当前选择有效，但尚不能使用技能，必须继续选择。 |
| `valid_complete_or_continue` | 当前选择有效，可以使用技能，也可以继续选择。 |
| `valid_complete` | 当前选择有效，可以使用技能，不能继续选择。 |

没有对应查询时使用默认方法：目标数量为零时返回 `valid_complete`，否则返回 `invalid`，适用于无需目标的技能。

## 异常

技能定义的检查查询抛出的异常会传递给调用方。

## 注意

查询参数仍含两个固定目标位置，未提供的位置补为 `std::monostate`，`target_count` 为采用的元素数量：`0` 检查空选择，`1` 检查第一目标，`2` 检查第二目标。省略 `targets` 即可询问是否允许不选目标、是否必须或可以继续选择。达到两个目标的上限后，查询应返回 `invalid` 或 `valid_complete`。

检查第二目标时，由调用方保证第一目标合法，查询可直接据此检查第二目标。检查不要求调用方曾经调用前一步，执行器不会补做前一步检查；修改第一目标或相关对局条件后，不能继续沿用原有的合法性结论。技能定义仍可拒绝不允许的目标数量。

本操作只查询本技能定义，不验证支付，也不计算费用、提交行动或修改牌桌。查询直接返回检查结果，不接收随机源或返回效果入口。库不记录检查进度，直接按本次 `targets` 的长度确定检查步骤。

本操作与 [`skill_payment_validate`](skill_payment_validate.md) 独立，且不要求先报价。调用方仍须保证传入当前现场内有效的候选索引，并在选择使用技能时保证目标与技能使用条件成立；[`use_skill`](use_skill.md) 不会自动检查。
