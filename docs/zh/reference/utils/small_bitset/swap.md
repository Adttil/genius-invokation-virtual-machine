# givm::small_bitset::swap

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr void swap(small_bitset& other) noexcept;
friend constexpr void swap(small_bitset& first, small_bitset& second) noexcept;
```

交换两个相同类型位集的位数和内容，支持内联存储与分配存储之间的交换。不分配存储，不抛异常，与自身交换保持原值。

可直接调用 `a.swap(b)`，也可通过参数相关查找使用 `swap(a, b)`。交换后，双方先前借出的视图及位代理不再保证有效。
