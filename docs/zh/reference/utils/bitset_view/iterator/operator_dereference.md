# givm::bitset_view::iterator::operator*

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr bool operator*() const noexcept;
```

读取当前位置的一位并返回独立 `bool` 值。当前位置必须可解引用，原位集的存储必须有效。
