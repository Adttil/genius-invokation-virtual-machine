[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_type_mask](../damage_type_mask.md) / **size**

# givm::damage_type_mask::size

定义于头文件 `<givm/enums/damage_type.hpp>`。

```cpp
constexpr std::size_t size() const noexcept;
```

返回可表示的伤害类型总数 `10`。

物理伤害、穿透伤害和真实伤害均占用自己的集合位。
