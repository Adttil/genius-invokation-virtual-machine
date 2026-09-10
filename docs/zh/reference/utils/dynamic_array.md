[givm](../../reference.md) / [通用工具](../utils.md) / **dynamic_array**

# givm::dynamic_array

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class T>
inline constexpr /* 可调用对象 */ dynamic_array{};
```

指定压入栈中的动态长度数组。可以给出元素个数预留空间，也可以提供一个范围作为初始内容。

## 模板参数

|  |  |
| --- | --- |
| `T` | 数组元素类型，须可平凡复制且满足栈的对齐要求 |

## 调用方式

`dynamic_array<T>(count)` 接受无符号整数数量；`dynamic_array<T>(range)` 接受元素类型为 `T` 的范围。结果用于 [`frame_stack::push`](frame_stack/push.md)，数组须放在帧的固定元素之前。

## 注意

数量或范围长度须可由 [`stack_count_t`](stack_count_t.md) 表示；数组和帧所需的总字节数也不得超出可表示的容量。

仅提供数量时不初始化数组元素。通过范围初始化时，范围须在压栈时有效，且能够完整遍历并取得一致的长度。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto [values] = stack.push(givm::dynamic_array<int>(3u));
    values[0] = 2;
    values[1] = 4;
    values[2] = 6;
    std::println("数组长度: {}", values.size());
    std::println("元素之和: {}", values[0] + values[1] + values[2]);
}
```

输出

```text
数组长度: 3
元素之和: 12
```
