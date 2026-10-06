[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **any**

# givm::elemental_reaction_mask::any

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr bool any() const noexcept;
```

集合至少包含一个值时返回 true。

`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
