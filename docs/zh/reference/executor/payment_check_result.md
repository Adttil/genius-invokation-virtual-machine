[givm](../../reference.md) / [执行](../executor.md) / **payment_check_result**

# givm::payment_check_result

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class payment_check_result : std::uint8_t
{
    valid,
    requirement_mismatch,
    insufficient_dice
};
```

一次骰子支付检查的结果，区分费用不匹配与持有数量不足。

## 枚举值

| | |
| --- | --- |
| `valid` | 所选骰子符合费用要求，且持有数量足够 |
| `requirement_mismatch` | 所选骰子的种类或总数不符合费用要求 |
| `insufficient_dice` | 所选骰子符合费用要求，但持有数量不足 |

## 注意

检查先判断费用是否匹配，再判断持有数量。两者均不满足时返回 `requirement_mismatch`。

## 参阅

| | |
| --- | --- |
| [`execution_view<action_selection>::check_switch_payment`](execution_view/action_selection/check_switch_payment.md) | 检查所选骰子能否支付切换至指定角色的费用 |
