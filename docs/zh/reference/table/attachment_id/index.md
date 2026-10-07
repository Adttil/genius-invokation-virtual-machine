[givm](../../../reference.md) / [牌桌](../../table.md) / [attachment_id](../attachment_id.md) / **index**

# givm::attachment_id::index

```cpp
constexpr std::uint32_t index() const noexcept;
```

取得最后一级的索引，与组合构造传入的 index 一致。卡牌状态使用全桌状态槽位索引；反应 ID 返回反应槽位的整数编码。
