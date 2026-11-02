[givm](../../reference.md) / [通用工具](../utils.md) / **bitset_view**

# givm::bitset_view

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
class bitset_view;
```

借用一组布尔标志或它们的前缀，只提供读取，不拥有存储。通过 [`small_bitset::view`](small_bitset/view.md) 或 [`small_bitset::take`](small_bitset/take.md) 取得，类型与原位集的内联容量无关。

| | |
| --- | --- |
| [构造函数](bitset_view/constructors.md) | 默认构造空视图，或复制另一份借用 |
| [`iterator`](bitset_view/iterator.md) | 解引用返回 `bool` 的只读随机访问迭代器 |
| [`size`](bitset_view/size.md) | 取得可读取的位数 |
| [`empty`](bitset_view/empty.md) | 判断长度是否为零 |
| [`operator[]`](bitset_view/operator_at.md) | 按位置读取一位 |
| [`take`](bitset_view/take.md) | 取得更短的前缀借用 |
| [`begin`](bitset_view/begin.md)、[`end`](bitset_view/end.md) | 遍历视图中的位 |

创建视图时已取得数据地址，随后读取各位不再区分内联或分配存储。原位集的修改会通过视图反映；原位集析构、被移出、交换或重新赋值后，旧视图不再保证有效。
