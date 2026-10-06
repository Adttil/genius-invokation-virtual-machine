[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **size**

# givm::elemental_reaction_mask::size

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr std::size_t size() const noexcept;
```

返回可表示的反应槽位总数 `17`。

`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
