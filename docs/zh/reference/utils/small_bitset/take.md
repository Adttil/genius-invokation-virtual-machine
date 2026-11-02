# givm::small_bitset::take

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr bitset_view take(std::size_t count) const noexcept;
```

借用从第零位开始的 `count` 位，等价于 `view().take(count)`，不复制或分配。`count` 为零时取得空视图。

要求 `count <= size()`。Debug 构建断言此前提，Release 不检查。借用的生命周期规则见 [`view`](view.md)。
