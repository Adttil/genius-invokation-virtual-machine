[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_definition_id](../variant_definition_id.md) / **value**

# givm::variant_definition_id::value

```cpp
constexpr std::uint64_t value() const noexcept;
```

返回包含类别及索引的完整编码字，可供整数构造恢复相同的 ID。它不是单独的区域索引；需要索引时先提取单类别 ID，再调用相应的访问函数。
