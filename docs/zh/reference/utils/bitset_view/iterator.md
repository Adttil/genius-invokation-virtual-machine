# givm::bitset_view::iterator

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
class bitset_view::iterator;
```

按位置读取位数据的随机访问迭代器，解引用和下标访问均返回 `bool` 值，不提供写入操作。通过 [`begin`](begin.md)、[`end`](end.md) 取得。

`value_type`、`reference` 为 `bool`，`difference_type` 为 `std::ptrdiff_t`，`pointer` 为 `void`；`iterator_category` 和 `iterator_concept` 均为 `std::random_access_iterator_tag`。

| | |
| --- | --- |
| [构造函数](iterator/constructors.md) | 默认构造或复制一个位置 |
| [`operator*`](iterator/operator_dereference.md) | 读取当前位置 |
| [`operator[]`](iterator/operator_at.md) | 按相对偏移读取 |
| [`operator++`](iterator/increment.md) | 前进一位 |
| [`operator--`](iterator/decrement.md) | 后退一位 |
| [`operator+=`、`operator-=`](iterator/advance.md) | 调整当前位置 |
| [加减运算](iterator/arithmetic.md) | 取得偏移后的位置或计算距离 |
| [比较运算](iterator/comparison.md) | 判断位置相等或先后 |

迭代器不依赖视图对象本身，只借用原位集的数据。位移结果必须是同一范围内的有效位置或末尾；距离必须能由 `difference_type` 表示。原位集析构、被移出、交换或重新赋值后，旧迭代器不再保证有效。
