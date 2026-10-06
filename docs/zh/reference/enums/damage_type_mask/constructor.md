[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_type_mask](../damage_type_mask.md) / **构造函数**

# givm::damage_type_mask::damage_type_mask

定义于头文件 `<givm/enums/damage_type.hpp>`。

```cpp
constexpr damage_type_mask() noexcept;
constexpr damage_type_mask(damage_type type) noexcept;
```

构造空集合，或只含指定伤害类型的集合。单值构造允许隐式转换。

`type` 应为有效的 `damage_type` 值。
物理伤害、穿透伤害和真实伤害均占用自己的集合位。
