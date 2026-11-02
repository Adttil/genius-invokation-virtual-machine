# givm::bitset_view::empty

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr bool empty() const noexcept;
```

返回 `size() == 0`。所有可读取位的值均为零，不表示视图为空。
