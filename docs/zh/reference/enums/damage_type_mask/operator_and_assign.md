[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_type_mask](../damage_type_mask.md) / **operator&=**

# givm::damage_type_mask::operator&=

定义于头文件 `<givm/enums/damage_type.hpp>`。

```cpp
constexpr damage_type_mask& operator&=(const damage_type_mask& other) noexcept;
```

将当前集合更新为与 other 的交集，返回 `*this`。
