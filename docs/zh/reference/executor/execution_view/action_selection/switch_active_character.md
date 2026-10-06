[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **switch_active_character**

# givm::execution_view<execution_state::action_selection>::switch_active_character

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class TRandom>
execution_state switch_active_character(
    const definition_library& library, table& card_table, TRandom& random_source,
    std::size_t target_index, const dice_counts& paid_dice
) const;
```

对具体的切换角色选择报价，并立即采用结果推进行动。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 可调用并返回 `std::uint32_t` 的随机源类型。 |

## 参数

| | |
| --- | --- |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `random_source` | 后续执行使用的随机源；报价不消耗随机数。 |
| `target_index` | 从零开始的当前行动候选索引。 |
| `paid_dice` | 原选支付骰子的颜色及数量。 |

## 返回值

推进后到达的执行现场。

## 异常

Debug 检查现场、候选或报价标识的有效性，并以结构化异常报告误用。费用响应抛出的异常直接传递；失败报价不能采用或在当前窗口重算。Release 不执行这些输入校验。

## 注意

相当于先调用 `calculate_switch_cost`，再使用返回标识调用 `switch_active_character_with_cached_cost`。只用于尚未报价的操作与目标组合。需要预览费用、检查支付或修正输入时，先显式报价并保存标识。

如果直接接口在完成报价后抛出 Debug 支付异常，它不会返回报价标识，也不回滚报价；调用方不能在本窗口重算这个组合。
