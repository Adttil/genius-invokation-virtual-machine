[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **target_validation**

# givm::target_validation

定义于头文件 `<givm/definition.hpp>`

```cpp
enum class target_validation : std::uint8_t
{
    invalid,
    valid_incomplete,
    valid_complete_or_continue,
    valid_complete
};
```

目标选择合法性检查的结果，同时说明当前选择能否完成或继续追加目标。

## 枚举值

| | |
| --- | --- |
| `invalid` | 本步目标选择或相关条件不合法。 |
| `valid_incomplete` | 当前选择有效，但尚不能完成，必须继续选择。 |
| `valid_complete_or_continue` | 当前选择有效，可以完成，也可以继续选择。 |
| `valid_complete` | 当前选择有效，可以完成，不能继续选择。 |

空选择同样可以检查，用于区分必须选目标、允许不选但可继续选择，以及无需目标且不可继续选择。是否允许空选择、目标数量上限和其他合法性条件由具体查询决定。

当前用于 [`card_target_validation`](card_target_validation.md)，表达出牌目标与用牌条件的分步检查结果。
