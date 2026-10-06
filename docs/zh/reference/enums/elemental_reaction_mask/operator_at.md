[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **operator[]**

# givm::elemental_reaction_mask::operator[]

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr bool operator[](elemental_reaction type) const noexcept;
```

查询集合是否记录了 `type` 指定的反应槽位。

`type` 应为有效的 `elemental_reaction` 值。
`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
