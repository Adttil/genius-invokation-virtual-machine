# givm::bitset_view::iterator::operator[]

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr bool operator[](difference_type offset) const noexcept;
```

读取相对于当前位置偏移 `offset` 位的值，等价于 `*(*this + offset)`。偏移可以为负，目标位置必须可解引用。
