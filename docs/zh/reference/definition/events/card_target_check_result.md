[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_target_check_result**

# givm::card_target_check_result

定义于头文件 `<givm/definition.hpp>`

```cpp
enum class card_target_check_result : std::uint8_t
{
    invalid,
    valid_incomplete,
    valid_complete_or_continue,
    valid_complete
};
```

牌定义检查当前已选目标与用牌条件后给出的结果，同时说明能否完成或继续选择。

## 枚举值

| | |
| --- | --- |
| `invalid` | 本步目标或用牌条件不合法。 |
| `valid_incomplete` | 当前选择有效，但尚不能出牌，必须继续选择。 |
| `valid_complete_or_continue` | 当前选择有效，可以出牌，也可以继续选择。 |
| `valid_complete` | 当前选择有效，可以出牌，不能继续选择。 |

检查由 [`card_target_check`](card_target_check.md) 的本牌响应决定，已选数量由事件的 `target_count` 指明。空选择同样可以检查，用于区分必须选目标、允许不选但可继续选择，以及无需目标且不可继续选择。达到两个目标的上限后，应返回 `invalid` 或 `valid_complete`。

无需目标检查的牌省略响应即可，结果为 `valid_complete`。未使用的目标位置忽略；此结果不涉及骰子支付。
