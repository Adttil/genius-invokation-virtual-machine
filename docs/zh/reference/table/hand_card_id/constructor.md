[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_id](../hand_card_id.md) / **constructor**

# givm::hand_card_id::constructor

```cpp
constexpr hand_card_id() noexcept = default;
constexpr hand_card_id(player_id parent_id, std::uint32_t index);
```

默认构造保持平凡，未初始化的 ID 须先被赋予完整 ID。组合构造保存父级身份与末级索引；Debug 检查玩家及父级索引的编码前提。
