[givm](../../../reference.md) / [牌桌](../../table.md) / [optional_tag_id](../optional_tag_id.md) / **get**

# givm::optional_tag_id::get

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr tag_id get() const;
constexpr tag_id operator*() const;
```

取得非空标签 ID。Debug 检查空值，失败抛出 std::invalid_argument。
