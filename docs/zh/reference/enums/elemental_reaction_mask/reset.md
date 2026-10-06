[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **reset**

# givm::elemental_reaction_mask::reset

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr elemental_reaction_mask& reset() noexcept;
constexpr elemental_reaction_mask& reset(elemental_reaction type) noexcept;
```

无参版本清空集合；带参数版本清除指定值。返回 `*this`。

`type` 应为有效的 `elemental_reaction` 值。
`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
