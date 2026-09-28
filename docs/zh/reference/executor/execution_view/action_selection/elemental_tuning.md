[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **elemental_tuning**

# givm::execution_view<execution_state::action_selection>::elemental_tuning

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
execution_state elemental_tuning(
    const definition_library& library, table& card_table, TRandom& random,
    std::size_t card_index, elemental_dice from
) const;
```

选择一张手牌与一枚元素骰进行元素调和。提交后立即推进，手牌离场，骰子转换为当前出战角色的元素；转换结果可以被调和修饰事件改变。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 非 `const`、非 `volatile` 的可调用对象类型，其无参数调用结果可隐式转换为 `std::uint32_t`。 |

## 参数

| | |
| --- | --- |
| `library` | 与当前执行现场配套的定义库。 |
| `card_table` | 当前对局牌桌。 |
| `random` | 本次推进使用的随机源，以左值传入。 |
| `card_index` | 当前手牌候选索引，须小于 [`card_count()`](card_count.md)。 |
| `from` | 选择转换的骰子元素。 |

## 返回值

推进后到达的 [`execution_state`](../../execution_state.md)，可能是下一处输入、观察或终局现场。

## 异常

未定义 `NDEBUG` 时，现场已失效或种类错误会抛出 [`execution_view_error`](../../execution_view_error.md)；输入合法性结果失败时抛出 [`view_input_error`](../../view_input_error.md)；实体 ID 越界或已移除的诊断沿用 [`command_input_error`](../../command_input_error.md)。这些检查均在填写选择和开始推进之前。进入执行后，定义源、随机源及命令检查产生的异常向外传播，执行不提供回滚。

## 注意

本操作提交并推进，Debug 自动检查输入，Release 由调用方保证输入合法。调用方须保证该牌允许调和、选择的是持有的基础元素骰，且与出战角色自身元素不同；可分别使用 [`elemental_tuning_card_validate`](elemental_tuning_card_validate.md) 和 [`elemental_tuning_dice_validate`](elemental_tuning_dice_validate.md) 检查。

调和与出牌共享 [`card_count`](card_count.md)、[`card_id`](card_id.md) 提供的手牌候选。无需计算出牌费用，也不采用出牌的支付或目标检查。

执行先广播 [`elemental_tuning_modification`](../../../definition/events/elemental_tuning_modification.md)，此时卡牌及附属状态仍然有效。修饰响应结算完成后，按最终 `to` 转换一枚骰子并移除手牌，再广播 [`elemental_tuning_completed`](../../../definition/events/elemental_tuning_completed.md)。转换不触发骰子产生或消耗通知，也不触发出牌原效果及出牌广播。

调和后仍由当前玩家行动，重新处理 [`before_action`](../../../definition/events/before_action.md) 并进入新的行动选择现场。旧现场的候选与报价不再适用。
