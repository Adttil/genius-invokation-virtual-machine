[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_type_mask](../damage_type_mask.md) / **operator|**

# givm::operator|

定义于头文件 `<givm/enums/damage_type.hpp>`。

```cpp
constexpr damage_type_mask operator|(damage_type_mask lhs, const damage_type_mask& rhs) noexcept;
constexpr damage_type_mask operator|(damage_type lhs, damage_type rhs) noexcept;
```

取得两个集合的并集，不修改实参。支持两个集合，或集合与一个枚举值的混合运算。此外支持两个枚举值直接取并集。
