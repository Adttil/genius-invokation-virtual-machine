# givm::small_bitset::inline_capacity

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
static constexpr std::size_t inline_capacity;
```

实际内联容量，为模板实参 `InlineBits` 按 64 位向上取整后的值。实际位数不超过此值时不分配存储；超过时整个位集使用分配的存储。`InlineBits` 为 `0` 时该值也为 `0`。
