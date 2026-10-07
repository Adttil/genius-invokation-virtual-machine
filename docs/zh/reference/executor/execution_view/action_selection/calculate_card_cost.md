[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **calculate_card_cost**

# givm::execution_view<execution_state::action_selection>::calculate_card_cost

定义于头文件 `<givm/runtime.hpp>`

```cpp
card_cost_id calculate_card_cost(const definition_library& library, const table& card_table,
    std::size_t card_index, std::span<const card_target_id> targets = {}) const;
```

对具体的出牌选择同步报价，并返回保存该报价的标识。

## 参数

| | |
| --- | --- |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `card_index` | 从零开始的当前行动候选索引。 |
| `targets` | 完整的目标选择；省略表示两个空槽。 |

## 返回值

当前行动窗口内有效的 [`card_cost_id`](../../card_cost_id.md)。可用 `card_cost(id)` 读取完整报价。

## 异常

Debug 检查现场、候选或报价标识的有效性，并以结构化异常报告误用。费用响应抛出的异常直接传递；失败报价不能采用或在当前窗口重算。Release 不执行这些输入校验。

## 注意

同一窗口内，同一操作和完整目标组合只报价一次。调用方保存返回的标识，之后可反复读取；Debug 检查重复报价，Release 不搜索去重。响应只处理编号 0，可修改费用和准备程序输入，但不执行程序、不修改牌桌、不使用随机数。

目标最多两个，超过两项的内容忽略。``null` 空类别` 表示空槽，遇到首个空槽即结束目标序列；报价无需额外保存目标数量。完整目标必须合法，分步检查仍由相应的 `*_targets_validate` 提供。

报价追加缓存但不推进行动。其他报价可能使借用的费用引用失效，标识仍能重新取得该结果。
