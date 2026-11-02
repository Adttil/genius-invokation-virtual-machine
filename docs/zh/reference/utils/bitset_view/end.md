# givm::bitset_view::end

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr iterator end() const noexcept;
```

返回视图最后一位之后的位置，不包含存储字中超出视图长度的位。末尾迭代器不可解引用，可用于距离计算和结束比较。
