[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **play_card**

# givm::execution_view<execution_state::action_selection>::play_card

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class TRandom>
execution_state play_card(
    const definition_library& library, table& card_table, TRandom& random_source,
    std::size_t card_index, const dice_counts& paid_dice, std::span<const card_target_id> targets = {}
) const;
```

对具体的出牌选择报价，并立即采用结果推进行动。

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
| `card_index` | 从零开始的当前行动候选索引。 |
| `paid_dice` | 原选支付骰子的颜色及数量。 |
| `targets` | 完整的目标选择；省略表示两个空槽。 |

## 返回值

推进后到达的执行现场。

## 异常

Debug 检查现场、候选或报价标识的有效性，并以结构化异常报告误用。费用响应抛出的异常直接传递；失败报价不能采用或在当前窗口重算。Release 不执行这些输入校验。

## 注意

相当于先调用 `calculate_card_cost`，再使用返回标识调用 `play_card_with_cached_cost`。只用于尚未报价的操作与目标组合。需要预览费用、检查支付或修正输入时，先显式报价并保存标识。

如果直接接口在完成报价后抛出 Debug 支付异常，它不会返回报价标识，也不回滚报价；调用方不能在本窗口重算这个组合。

目标最多两个，超过两项的内容忽略。`std::monostate` 表示空槽，遇到首个空槽即结束目标序列；报价无需额外保存目标数量。完整目标必须合法，分步检查仍由相应的 `*_targets_validate` 提供。
