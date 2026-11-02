# givm::bitset_view::bitset_view

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr bitset_view() noexcept;
```

创建不引用任何数据的空视图。视图可以正常复制和赋值，复制仅复制借用，不延长原位集的生命周期。
