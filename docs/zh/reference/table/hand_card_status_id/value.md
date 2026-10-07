[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_status_id](../hand_card_status_id.md) / **value**

# givm::hand_card_status_id::value

```cpp
constexpr std::uint64_t value() const noexcept;
```

只读取得内部保存的完整编码字，包含父级身份及末级索引。构造 ID 时使用相应的父级 ID 和索引。
