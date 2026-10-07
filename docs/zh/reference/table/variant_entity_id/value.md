[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_entity_id](../variant_entity_id.md) / **value**

# givm::variant_entity_id::value

```cpp
constexpr std::uint64_t value() const noexcept;
```

只读返回包含类别、父级身份及末级索引的完整编码字。取得末级索引时先提取单类别 ID，再调用 `index()`。
