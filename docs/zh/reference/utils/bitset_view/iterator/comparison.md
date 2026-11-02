# givm::bitset_view::iterator 的比较运算

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
friend constexpr bool operator==(const iterator&, const iterator&) noexcept;
friend constexpr auto operator<=>(const iterator& first, const iterator& second) noexcept;
```

相等比较同时比较数据来源及位位置；有序比较按位位置确定先后，并提供 `<`、`<=`、`>`、`>=`。

有序比较要求位置来自同一位集。Debug 构建断言此前提，Release 不检查。
