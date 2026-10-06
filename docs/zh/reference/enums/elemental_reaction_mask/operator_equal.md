[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **operator==**

# givm::elemental_reaction_mask::operator==

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr bool operator==(const elemental_reaction_mask& other) const noexcept;
```

比较两个集合所记录的值是否完全相同。

`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
