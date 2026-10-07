[givm](../../../reference.md) / [牌桌](../../table.md) / [optional_tag_id](../optional_tag_id.md) / **value**

# givm::optional_tag_id::value

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr std::size_t value() const noexcept;
```

取得标签索引，或空值所使用的 size_t 最大值。该原始值不能直接代替 get() 用于索引。
