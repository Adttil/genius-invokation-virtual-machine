# givm::small_bitset::set

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr void set(std::size_t index, bool value = true) noexcept;
```

将第 `index` 位设为 `value`，省略 `value` 时置位。操作不分配存储，不改变位数；借出的视图会读到修改后的值。

要求 `index < size()`。Debug 构建断言此前提，Release 不检查。
