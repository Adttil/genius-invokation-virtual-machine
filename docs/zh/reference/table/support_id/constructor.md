[givm](../../../reference.md) / [牌桌](../../table.md) / [support_id](../support_id.md) / **constructor**

# givm::support_id::constructor

```cpp
constexpr support_id() noexcept = default;
constexpr support_id(player_id parent_id, std::uint32_t index);
```

默认构造保持平凡，未初始化的 ID 须先被赋予完整 ID。组合构造保存父级身份与末级索引；Debug 检查玩家及父级索引的编码前提。
