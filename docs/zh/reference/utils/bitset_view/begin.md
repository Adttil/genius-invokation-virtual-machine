# givm::bitset_view::begin

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr iterator begin() const noexcept;
```

返回第零位的只读随机访问迭代器。空视图的起始迭代器等于末尾迭代器。

迭代器直接借用位数据，视图对象析构不影响它；原位集的存储仍须保持有效。
