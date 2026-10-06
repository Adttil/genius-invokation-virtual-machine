[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_type_mask](../damage_type_mask.md) / **set**

# givm::damage_type_mask::set

定义于头文件 `<givm/enums/damage_type.hpp>`。

```cpp
constexpr damage_type_mask& set() noexcept;
constexpr damage_type_mask& set(damage_type type, bool value = true) noexcept;
```

无参版本记录全部值。带参数版本在 value 为 true 时记录指定值，为 false 时清除它。返回 `*this`。

`type` 应为有效的 `damage_type` 值。
