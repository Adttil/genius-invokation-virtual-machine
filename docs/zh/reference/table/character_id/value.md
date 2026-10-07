[givm](../../../reference.md) / [牌桌](../../table.md) / [character_id](../character_id.md) / **value**

# givm::character_id::value

```cpp
constexpr std::uint64_t value() const noexcept;
```

只读取得内部保存的完整编码字，包含父级身份及末级索引。构造 ID 时使用相应的父级 ID 和索引。
