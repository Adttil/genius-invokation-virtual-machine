[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_type_mask](../damage_type_mask.md) / **operator[]**

# givm::damage_type_mask::operator[]

定义于头文件 `<givm/enums/damage_type.hpp>`。

```cpp
constexpr bool operator[](damage_type type) const noexcept;
```

查询集合是否记录了 `type` 指定的伤害类型。

`type` 应为有效的 `damage_type` 值。
物理伤害、穿透伤害和真实伤害均占用自己的集合位。
