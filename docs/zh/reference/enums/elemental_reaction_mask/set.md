[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **set**

# givm::elemental_reaction_mask::set

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr elemental_reaction_mask& set() noexcept;
constexpr elemental_reaction_mask& set(elemental_reaction type, bool value = true) noexcept;
```

无参版本记录全部值。带参数版本在 value 为 true 时记录指定值，为 false 时清除它。返回 `*this`。

`type` 应为有效的 `elemental_reaction` 值。
`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
