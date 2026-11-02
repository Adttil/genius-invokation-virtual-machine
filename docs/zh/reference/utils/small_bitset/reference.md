# givm::small_bitset::reference

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
class reference;
```

单个位的可写代理，由可写位集的 [`operator[]`](operator_at.md) 返回。它借用原位集中的一位，支持读取为 `bool` 和写入新值，不拥有存储。

| | |
| --- | --- |
| [构造函数](reference/constructors.md) | 复制另一份位借用 |
| [`operator bool`](reference/operator_bool.md) | 读取当前值 |
| [`operator=`](reference/operator_assign.md) | 修改所引用位的值 |

`auto bit = flags[index]` 保留借用，后续修改该位会影响 `bit` 的读取结果；`bool value = flags[index]` 则保存独立值。原位集析构、被移出、交换或重新赋值后，旧代理不再保证有效。
