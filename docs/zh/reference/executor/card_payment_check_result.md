[givm](../../reference.md) / [执行](../executor.md) / **card_payment_check_result**

# givm::card_payment_check_result

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class card_payment_check_result : std::uint8_t
{
    valid,
    requirement_mismatch,
    insufficient_dice
};
```

出牌的骰子支付检查结果，区分费用不匹配与持有数量不足。

## 枚举值

| | |
| --- | --- |
| `valid` | 所选骰子符合费用要求，且持有数量足够。 |
| `requirement_mismatch` | 所选骰子的种类或总数不符合费用要求。 |
| `insufficient_dice` | 所选骰子符合费用要求，但持有数量不足。 |

## 注意

[`check_card_payment`](execution_view/action_selection/check_card_payment.md) 先检查费用匹配，失败时立即返回 `requirement_mismatch`；匹配后才检查持有数量。此结果不表示牌的目标或其他用牌条件合法。

