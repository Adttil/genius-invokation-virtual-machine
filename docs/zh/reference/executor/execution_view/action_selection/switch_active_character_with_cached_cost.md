[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **switch_active_character_with_cached_cost**

# givm::execution_view<execution_state::action_selection>::switch_active_character_with_cached_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
execution_state switch_active_character_with_cached_cost(
    const definition_library& library, table& card_table, TRandom& random,
    std::size_t target_index, const dice_counts& paid_dice
) const;
```

采用已经完整计算的费用与对应支付效果，提交行动并推进；不重新报价。参数、返回现场及执行顺序见 [`switch_active_character`](switch_active_character.md)。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 可无参数调用、结果可转换为 `std::uint32_t` 的随机源类型。 |

## 参数

| | |
| --- | --- |
| `library` | 与当前执行现场配套的定义库。 |
| `card_table` | 当前对局的牌桌。 |
| `random` | 本次推进使用的随机源，以左值传入。 |
| `paid_dice` | 本次支付的骰子数量。 |
| `target_index` | 当前现场的候选索引。 |

## 返回值

推进后到达的 [`execution_state`](../../execution_state.md)，可能是下一处输入、观察或终局现场。

## 异常

Debug 检查视图有效性、报价状态和行动参数，并可能抛出 [`execution_view_error`](../../execution_view_error.md) 或 [`view_input_error`](../../view_input_error.md)。Release 由调用方保证这些前提。开始推进后执行异常向外传播，不提供回滚。
