[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **switch_active_character**

# givm::execution_view<execution_state::action_selection>::switch_active_character

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
execution_state switch_active_character(
    const definition_library& library, table& card_table, TRandom& random,
    std::size_t target_index, const dice_counts& paid_dice
) const;

template<class TRandom>
execution_state switch_active_character_with_cached_cost(
    const definition_library& library, table& card_table, TRandom& random,
    std::size_t target_index, const dice_counts& paid_dice
) const;
```
[`dice_counts`](../../../enums/dice_counts.md)
[`definition_library`](../../definition_library.md)
[`table`](../../../table/table.md)

选择要切换至的角色和支付的骰子。可采用该角色已经计算的切换费用，也可在选择时同步计算报价。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 非 `const`、非 `volatile` 的可调用对象类型，其无参数调用结果可隐式转换为 `std::uint32_t`。 |

## 参数

| | |
| --- | --- |
| `target_index` | 从零开始的切换候选索引，须小于 [`switch_target_count()`](switch_target_count.md)。 |
| `paid_dice` | 本次实际支付的各类骰子数量，须满足采用的费用及持有数量。 |
| `library` | 与当前执行现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `random` | 本次推进使用的随机源，以左值传入。 |

## 返回值

推进后到达的 [`execution_state`](../../execution_state.md)，可能是下一处输入、观察或终局现场。

## 异常

未定义 `NDEBUG` 时，现场已失效或种类错误会抛出 [`execution_view_error`](../../execution_view_error.md)；输入合法性结果失败时抛出 [`view_input_error`](../../view_input_error.md)；实体 ID 越界或已移除的诊断沿用 [`command_input_error`](../../command_input_error.md)。这些检查均在填写选择和开始推进之前。进入执行后，定义源、随机源及命令检查产生的异常向外传播，执行不提供回滚。

计算费用时的响应异常也向外传播。报价失败后，该候选在本现场不能重新报价或使用缓存。

## 注意

若即时报价已经成功，而后续 Debug 输入检查失败，报价仍然保留；修正输入后应调用 `switch_active_character_with_cached_cost`，不能重新报价。

主动切换不受 `control` 或 `control_immunity` 附属限制；免控保护阻止的是 [`set_active_character`](../../../definition/commands/set_active_character.md) 和超载等非主动效果切人。

同一行动窗口内，每个候选只允许计算一次费用。`switch_active_character` 同步计算报价后提交，只用于尚未报价的候选；`switch_active_character_with_cached_cost` 使用已经完整计算的报价与对应支付效果，不重新计算。Debug 检查报价状态，Release 由调用方保证。

`switch_active_character_with_cached_cost` 采用已计算费用。调用方须先通过 [`calculate_switch_cost`](calculate_switch_cost.md) 为该候选完整报价，并自行保证该报价可用；本操作不重新计算费用。

`switch_active_character` 同步计算切换费用，成功后提交并推进。

两种方式均在 Debug 下检查支付是否合法。可先报价并调用 [`switch_payment_validate`](switch_payment_validate.md)，再用 `switch_active_character_with_cached_cost` 采用该费用；Release 不重复执行检查。

提交后立即推进，执行已确认的费用效果、扣除骰子与出战角色充能、依次处理骰子移除与充能变化通知，再完成切换；不会再次报价。开始推进后旧视图失效。
