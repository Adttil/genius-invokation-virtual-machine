[givm](../../reference.md) / [执行](../executor.md) / **switch_payment_check_result**

# givm::switch_payment_check_result

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class switch_payment_check_result : std::uint8_t
{
    valid,
    requirement_mismatch,
    insufficient_dice
};
```

切换出战角色时的骰子支付检查结果，区分费用不匹配与持有数量不足。

## 枚举值

| | |
| --- | --- |
| `valid` | 所选骰子符合费用要求，且持有数量足够 |
| `requirement_mismatch` | 所选骰子的种类或总数不符合费用要求 |
| `insufficient_dice` | 所选骰子符合费用要求，但持有数量不足 |

## 注意

检查先判断费用是否匹配，不匹配时立即返回 `requirement_mismatch`；匹配后再检查持有数量，不足时立即返回 `insufficient_dice`，全部通过时返回 `valid`。

## 参阅

| | |
| --- | --- |
| [`execution_view<action_selection>::check_switch_payment`](execution_view/action_selection/check_switch_payment.md) | 检查所选骰子能否支付切换至指定角色的费用 |
