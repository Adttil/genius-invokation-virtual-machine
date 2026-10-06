[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **operator&=**

# givm::elemental_reaction_mask::operator&=

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr elemental_reaction_mask& operator&=(const elemental_reaction_mask& other) noexcept;
```

将当前集合更新为与 other 的交集，返回 `*this`。

`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
