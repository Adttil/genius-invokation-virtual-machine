[givm](../../reference.md) / [通用工具](../utils.md) / **small_bitset**

# givm::small_bitset

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
template<std::size_t InlineBits>
class small_bitset;
```

保存一组按位置编号的布尔标志。小集合直接保存在对象中，超过内联容量时，整个位集使用一次独立分配的存储。

`InlineBits` 在 C++ 编译期指定，实际内联容量按 64 位向上取整；`0` 表示非空集合均使用分配的存储。实际位数在构造时指定，不因置位或清空而改变，不提供扩容操作。默认构造得到空位集。

## 成员

| | |
| --- | --- |
| [`inline_capacity`](small_bitset/inline_capacity.md) | 实际内联位数上限 |
| [`reference`](small_bitset/reference.md) | 可读取或赋值的单个位代理 |
| [构造函数](small_bitset/constructors.md) | 创建全部为零的位集，或复制、移入另一位集 |
| [`operator=`](small_bitset/operator_assign.md) | 整体替换位数和内容 |
| [`swap`](small_bitset/swap.md) | 交换两个位集的位数及内容 |
| [析构函数](small_bitset/destructor.md) | 释放拥有的存储 |
| [`size`](small_bitset/size.md) | 取得实际位数 |
| [`empty`](small_bitset/empty.md) | 判断是否没有任何位 |
| [`operator[]`](small_bitset/operator_at.md) | 按位置读取或赋值；只读对象返回 `bool`，可写对象返回位代理 |
| [`set`](small_bitset/set.md) | 设置或清除一位 |
| [`reset`](small_bitset/reset.md) | 清除所有位，保留位数 |
| [`view`](small_bitset/view.md) | 借用整个位集 |
| [`take`](small_bitset/take.md) | 借用指定长度的前缀 |

复制后的对象拥有独立内容；移动后的源对象为空，仍可重新赋值。存储分配失败时沿用标准分配异常，复制赋值失败时目标保持原值。

## 示例

```cpp
#include <print>
#include <givm/utils/small_bitset.hpp>

int main()
{
    givm::small_bitset<65> flags{ 128 };
    flags[64] = true;
    const auto prefix = flags.take(65);
    std::println("内联容量: {}", flags.inline_capacity);
    std::println("前缀末位: {}", prefix[64]);
    flags.reset();
    std::println("清空后的位数: {}", flags.size());
    std::println("前缀末位: {}", prefix[64]);
}
```

输出

```text
内联容量: 128
前缀末位: true
清空后的位数: 128
前缀末位: false
```
