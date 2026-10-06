[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_type_mask](../damage_type_mask.md) / **reset**

# givm::damage_type_mask::reset

定义于头文件 `<givm/enums/damage_type.hpp>`。

```cpp
constexpr damage_type_mask& reset() noexcept;
constexpr damage_type_mask& reset(damage_type type) noexcept;
```

无参版本清空集合；带参数版本清除指定值。返回 `*this`。

`type` 应为有效的 `damage_type` 值。
