[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **push**

# givm::frame_stack::push

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T>
constexpr auto push();

template<class... T>
    requires detail::dynamic_array_prefix<detail::stack_push_element_t<T>...>
constexpr auto push(const T&... t);
```

在栈顶加入一帧，随后可以通过返回的 view 读取或填写其中的值。

## 模板参数

|  |  |
| --- | --- |
| `T...` | 非空的元素类型序列，或用于初始化各元素的实参类型；值须可平凡复制且满足栈的对齐要求 |

## 参数

|  |  |
| --- | --- |
| `t...` | 各元素的初值；动态数组通过 `dynamic_array<T>` 指定，并须位于所有固定元素之前 |

## 返回值

新帧的可写 [`frame_view`](../frame_view.md)。无参数重载只预留固定大小元素的空间，不初始化元素；带参数重载复制给定初值。

## 注意

无参数重载不用于创建动态数组。按数量创建的动态数组也不初始化元素，应先写入再读取。

空间不足时栈会增长，原有 view、引用和指针失效。

## 示例

```cpp
#include <print>
#include <array>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    stack.push<int>().get<0>() = 6;
    std::println("填写的值: {}", stack.top<int>().get<0>());
    const std::array values{ 2, 4, 6 };
    auto [array, count] = stack.push(givm::dynamic_array<int>(values), 3);
    std::println("数组元素数: {}", array.size());
    std::println("最后一个元素: {}", array.back());
    std::println("随帧保存的数量: {}", count);
}
```

输出

```text
填写的值: 6
数组元素数: 3
最后一个元素: 6
随帧保存的数量: 3
```

## 参阅

|  |  |
| --- | --- |
| [`dynamic_array`](../dynamic_array.md) | 指定动态数组的来源 |
| [`top`](top.md) | 访问已有数据 |
