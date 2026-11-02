# givm::bitset_view::iterator::iterator

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr iterator() noexcept;
```

创建没有可解引用元素的默认迭代器。迭代器可正常复制和赋值，复制仅复制位置及数据借用，不拥有存储。
