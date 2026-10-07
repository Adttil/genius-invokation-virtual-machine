[givm](../../../reference.md) / [牌桌](../../table.md) / [definition_id](../definition_id.md) / **value**

# givm::definition_id::value

```cpp
constexpr std::uint64_t value() const noexcept;
```

返回定义在所属类别中的索引。只有配套定义库中的有效索引可用于读取定义；该函数不查询定义库。
