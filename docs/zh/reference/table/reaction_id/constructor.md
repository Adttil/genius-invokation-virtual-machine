[givm](../../../reference.md) / [牌桌](../../table.md) / [reaction_id](../reaction_id.md) / **constructor**

# givm::reaction_id::constructor

```cpp
constexpr reaction_id() noexcept = default;
constexpr reaction_id(player_id parent_id, elemental_reaction slot);
```

默认构造保持平凡，未初始化的 ID 须先被赋予完整 ID。组合构造保存父级身份与末级索引；Debug 检查玩家及父级索引的编码前提。
