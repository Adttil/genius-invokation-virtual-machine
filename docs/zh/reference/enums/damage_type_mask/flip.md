[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_type_mask](../damage_type_mask.md) / **flip**

# givm::damage_type_mask::flip

定义于头文件 `<givm/enums/damage_type.hpp>`。

```cpp
constexpr damage_type_mask& flip() noexcept;
constexpr damage_type_mask& flip(damage_type type) noexcept;
```

无参版本反转全部可表示的值；带参数版本只反转指定值。已记录的值被清除，未记录的值被加入。返回 `*this`。

`type` 应为有效的 `damage_type` 值。
物理伤害、穿透伤害和真实伤害均占用自己的集合位。
