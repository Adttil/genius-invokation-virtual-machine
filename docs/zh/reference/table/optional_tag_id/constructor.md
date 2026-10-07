[givm](../../../reference.md) / [牌桌](../../table.md) / [optional_tag_id](../optional_tag_id.md) / **(构造函数)**

# givm::optional_tag_id::(构造函数)

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr optional_tag_id() noexcept = default;
constexpr optional_tag_id(std::nullptr_t) noexcept;
constexpr optional_tag_id(tag_id id) noexcept;
```

默认或 nullptr 构造为空；标签 ID 可隐式转换。可通过赋值 nullptr 清空。
