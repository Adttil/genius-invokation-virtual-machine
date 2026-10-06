[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **operator^**

# givm::operator^

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr elemental_reaction_mask operator^(elemental_reaction_mask lhs, const elemental_reaction_mask& rhs) noexcept;
```

取得两个集合的对称差集，不修改实参。支持两个集合，或集合与一个枚举值的混合运算。

`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
