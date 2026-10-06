[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **use_technique_with_cached_cost**

# givm::execution_view<execution_state::action_selection>::use_technique_with_cached_cost

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class TRandom>
execution_state use_technique_with_cached_cost(const definition_library& library, table& card_table,
    TRandom& random_source, technique_cost_id quote, const dice_counts& paid_dice) const;
```

采用已有报价，锁定使用特技及支付选择并推进行动。

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
| `quote` | 本窗口的对应报价标识，包含行动来源、目标和费用。 |
| `paid_dice` | 原选支付骰子的颜色及数量。 |

## 返回值

推进后到达的执行现场。

## 异常

Debug 检查现场、候选或报价标识的有效性，并以结构化异常报告误用。费用响应抛出的异常直接传递；失败报价不能采用或在当前窗口重算。Release 不执行这些输入校验。

## 注意

不重新报价、不另收行动候选或目标。各缓存程序按响应顺序执行，并分别完成全部派生结算，返回编号忽略；之后逐色尽可能扣除原选数量，其他颜色或万能骰不补足。充能从确认时锁定的原付费角色饱和扣除，费用效果换人不会改换付费对象。费用效果终局则停止后续付款和行动。

付款通知报告实际变化，与实际行动按所属段处理。需要修改支付输入时，可在推进前继续使用已经保存的标识；推进后的副作用不回滚。
