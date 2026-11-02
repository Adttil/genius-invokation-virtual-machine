# givm::bitset_view::operator[]

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr bool operator[](std::size_t index) const noexcept;
```

读取视图中第 `index` 位，返回 `bool`。前缀视图仍从原位集第零位开始编号。

要求 `index < size()` 且原存储仍然有效。Debug 构建断言索引前提，Release 不检查；视图不自动检查借用的生命周期。
