[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **构造函数**

# givm::elemental_reaction_mask::elemental_reaction_mask

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr elemental_reaction_mask() noexcept;
constexpr elemental_reaction_mask(elemental_reaction type) noexcept;
```

构造空集合，或只含指定反应槽位的集合。单值构造允许隐式转换。

`type` 应为有效的 `elemental_reaction` 值。
`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
