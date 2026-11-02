# givm::bitset_view::iterator::operator--

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr iterator& operator--() noexcept;
constexpr iterator operator--(int) noexcept;
```

后退一位。前置形式返回修改后的自身引用，后置形式返回修改前的位置。结果必须是有效位置。
