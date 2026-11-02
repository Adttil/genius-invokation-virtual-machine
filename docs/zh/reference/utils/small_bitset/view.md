# givm::small_bitset::view

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr bitset_view view() const noexcept;
```

取得整个位集的只读借用，长度为 `size()`，不复制位数据。返回类型与内联容量无关。

对原位集调用 `set`、`reset` 或通过位代理赋值后，视图会读到修改后的内容。原位集析构、被移出、交换或重新赋值后，视图不再保证有效；从临时对象取得的视图不得在临时对象析构后使用。
