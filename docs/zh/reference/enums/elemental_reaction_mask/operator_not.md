[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **operator~**

# givm::operator~

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr elemental_reaction_mask operator~(elemental_reaction_mask value) noexcept;
```

取得补集，包含原集合没有记录的全部可表示值。不修改实参。

`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
