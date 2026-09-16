[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_target_check_result**

# givm::card_target_check_result

定义于头文件 `<givm/definition.hpp>`

```cpp
enum class card_target_check_result : std::uint8_t
{
    valid,
    invalid_first_target,
    invalid_second_target,
    unmet_condition
};
```

牌定义检查目标与用牌条件后给出的结果。

## 枚举值

| | |
| --- | --- |
| `valid` | 本牌所需的目标与用牌条件成立。 |
| `invalid_first_target` | 第一个目标不符合本牌要求。 |
| `invalid_second_target` | 第二个目标不符合本牌要求。 |
| `unmet_condition` | 其他用牌条件不满足。 |

检查由 [`card_target_check`](card_target_check.md) 的本牌响应决定，库不规定检查顺序，也不检查目标数量。未使用的目标位置忽略；此结果不涉及骰子支付。

