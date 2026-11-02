# givm::bitset_view::iterator 的加减运算

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
friend constexpr iterator operator+(iterator value, difference_type offset) noexcept;
friend constexpr iterator operator+(difference_type offset, iterator value) noexcept;
friend constexpr iterator operator-(iterator value, difference_type offset) noexcept;
friend constexpr difference_type operator-(const iterator& first, const iterator& second) noexcept;
```

迭代器与整数相加减取得新位置，不修改原位置。两个迭代器相减取得前者相对后者的位距离，支持负距离。

距离计算要求两个位置属于同一位集，且结果可由 `difference_type` 表示。Debug 构建断言数据来源相同，Release 不检查。
