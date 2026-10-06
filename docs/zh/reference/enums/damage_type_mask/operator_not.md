[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_type_mask](../damage_type_mask.md) / **operator~**

# givm::operator~

定义于头文件 `<givm/enums/damage_type.hpp>`。

```cpp
constexpr damage_type_mask operator~(damage_type_mask value) noexcept;
```

取得补集，包含原集合没有记录的全部可表示值。不修改实参。

物理伤害、穿透伤害和真实伤害均占用自己的集合位。
