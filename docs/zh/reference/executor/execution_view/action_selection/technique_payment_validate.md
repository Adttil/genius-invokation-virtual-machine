[givm](../../../../reference.md) / [行动选择](../action_selection.md) / **technique_payment_validate**

# technique_payment_validate

定义于头文件 `<givm/executor.hpp>`

```cpp
technique_payment_validation technique_payment_validate(const table& card_table, const dice_counts& paid_dice) const noexcept(/* Release 为 true，Debug 为 false */);
```

检查已经计算的特技报价。依次检查骰子费用匹配、持有骰子数量、充能标签、出战角色的充能数量；第一次失败立即返回相应枚举值。检查不提交行动，也不检查受控与目标。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
