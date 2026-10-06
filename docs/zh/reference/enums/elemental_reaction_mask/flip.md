[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **flip**

# givm::elemental_reaction_mask::flip

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr elemental_reaction_mask& flip() noexcept;
constexpr elemental_reaction_mask& flip(elemental_reaction type) noexcept;
```

无参版本反转全部可表示的值；带参数版本只反转指定值。已记录的值被清除，未记录的值被加入。返回 `*this`。

`type` 应为有效的 `elemental_reaction` 值。
`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
