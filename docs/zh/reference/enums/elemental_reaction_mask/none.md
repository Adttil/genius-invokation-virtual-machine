[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **none**

# givm::elemental_reaction_mask::none

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr bool none() const noexcept;
```

集合为空时返回 true。

`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
