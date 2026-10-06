[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_reaction_mask](../elemental_reaction_mask.md) / **count**

# givm::elemental_reaction_mask::count

定义于头文件 `<givm/enums/elemental_reaction.hpp>`。

```cpp
constexpr std::size_t count() const noexcept;
```

取得当前集合中已记录的值数量。

`elemental_reaction::none` 不占集合位；查询返回 false，设置、清除和翻转该值不改变集合。
