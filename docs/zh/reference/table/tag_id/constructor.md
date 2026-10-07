[givm](../../../reference.md) / [牌桌](../../table.md) / [tag_id](../tag_id.md) / **(构造函数)**

# givm::tag_id::(构造函数)

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr tag_id() noexcept = default;
constexpr explicit tag_id(std::size_t index);
```

默认构造保持平凡。整数构造只在 Debug 拒绝空值编码，不检查标签是否属于某个定义库。
