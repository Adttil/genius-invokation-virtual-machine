# givm::small_bitset::size

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr std::size_t size() const noexcept;
```

返回实际位数，包含值为零的位。该值可以大于内联容量。
