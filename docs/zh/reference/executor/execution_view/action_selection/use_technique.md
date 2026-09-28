[givm](../../../../reference.md) / [行动选择](../action_selection.md) / **use_technique**

# givm::execution_view<execution_state::action_selection>::use_technique

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
execution_state use_technique(
    const definition_library& library, table& card_table, TRandom& random,
    const dice_counts& paid_dice, std::span<const technique_target_id> targets = {}
) const;

template<class TRandom>
execution_state use_technique_with_cached_cost(
    const definition_library& library, table& card_table, TRandom& random,
    const dice_counts& paid_dice, std::span<const technique_target_id> targets = {}
) const;
```

选择当前特技装备，提交支付骰子与至多两个目标，并推进到下一处暂停现场。

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
| `paid_dice` | 本次支付的骰子，须满足报价及持有数量。 |
| `targets` | 按顺序提供的目标，默认空 span；最多采用前两个，其余忽略。 |

## 返回值

推进后到达的 [`execution_state`](../../execution_state.md)，可能是下一处输入、观察或终局现场。

## 异常

未定义 `NDEBUG` 时，现场已失效或种类错误会抛出 [`execution_view_error`](../../execution_view_error.md)；输入合法性结果失败时抛出 [`view_input_error`](../../view_input_error.md)；实体 ID 越界或已移除的诊断沿用 [`command_input_error`](../../command_input_error.md)。这些检查均在填写选择和开始推进之前。进入执行后，定义源、随机源及命令检查产生的异常向外传播，执行不提供回滚。

## 注意

若即时报价已经成功，而后续 Debug 输入检查失败，报价仍然保留；修正输入后应调用 `use_technique_with_cached_cost`，不能重新报价。

`use_technique` 同步计算费用并提交；`use_technique_with_cached_cost` 使用已经完整计算的费用与对应支付效果，不重新报价。每个特技在一个行动现场只能计算一次费用。

调用方须保证存在特技、出战角色未受控、支付与目标合法。Debug 自动检查这些条件，Release 不检查。目标在 Debug 下按前缀逐步验证，最终选择须允许完成。

充能按报价从支付时的出战角色扣除。支付响应结算后广播 [`technique_will_be_used`](../../../definition/events/technique_will_be_used.md)，未取消时单播 [`technique_effect`](../../../definition/events/technique_effect.md)；最后总是广播 [`technique_used`](../../../definition/events/technique_used.md)。行动是否交给对方由最终速度决定。
