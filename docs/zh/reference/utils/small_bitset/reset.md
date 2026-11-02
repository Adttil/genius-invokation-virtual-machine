# givm::small_bitset::reset

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr void reset() noexcept;
```

将所有位设为零，保留位数和已有存储。操作不分配存储，已有视图仍然有效并会读到修改后的值。
