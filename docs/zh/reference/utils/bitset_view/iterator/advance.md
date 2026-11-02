# givm::bitset_view::iterator 的复合位移运算

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr iterator& operator+=(difference_type offset) noexcept;
constexpr iterator& operator-=(difference_type offset) noexcept;
```

分别前进或后退 `offset` 位并返回 `*this`。偏移可以为负；结果必须是同一范围内的有效位置或末尾。
