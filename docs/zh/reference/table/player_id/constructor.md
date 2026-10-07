[givm](../../../reference.md) / [牌桌](../../table.md) / [player_id](../player_id.md) / **constructor**

# givm::player_id::constructor

```cpp
constexpr player_id() noexcept = default;
constexpr explicit player_id(std::uint32_t index) noexcept;
```

默认构造保持平凡，未初始化的 ID 须先被赋予完整 ID。索引构造保存玩家索引。
