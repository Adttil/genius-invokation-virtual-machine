# givm::small_bitset::operator[]

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr bool operator[](std::size_t index) const noexcept;
constexpr reference operator[](std::size_t index) noexcept;
```

只读对象返回第 `index` 位的独立 `bool` 值。可写对象返回 [`reference`](reference.md) 代理，支持 `bool value = bits[index]`、`bits[index] = true` 以及 `bits[index] = bits[other]`。也可通过 [`set`](set.md) 修改。

要求 `index < size()`。Debug 构建断言此前提，Release 不检查。
