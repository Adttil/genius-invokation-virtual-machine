[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **switch_payment_validate**

# givm::execution_view<execution_state::action_selection>::switch_payment_validate

定义于头文件 `<givm/runtime.hpp>`

```cpp
constexpr switch_payment_validation switch_payment_validate(
    const table& card_table, switch_cost_id id, const dice_counts& paid_dice
) const noexcept(/* Release 为 true，Debug 为 false */);
```

检查选中骰子是否匹配已报价费用、是否持有这些骰子，以及当前出战角色的充能类型和数量。

## 参数

| | |
| --- | --- |
| `card_table` | 当前行动发生的牌桌。 |
| `id` | 本窗口的对应报价标识。 |
| `paid_dice` | 按颜色和数量选择的支付骰子。 |

## 返回值

`switch_payment_validation` 检查结果。

## 异常

Debug 检查现场、候选或报价标识的有效性，并以结构化异常报告误用。费用响应抛出的异常直接传递；失败报价不能采用或在当前窗口重算。Release 不执行这些输入校验。

## 注意

本操作只读取报价和牌桌，不执行费用效果或推进。Debug 提交自动执行支付检查，Release 由调用方保证输入有效。确认时付得起不表示费用效果结束后资源仍足够，实际付款采用逐色及充能饱和扣除。
