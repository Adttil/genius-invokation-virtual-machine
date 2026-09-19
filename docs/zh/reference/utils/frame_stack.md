[givm](../../reference.md) / [通用工具](../utils.md) / **frame_stack**

# givm::frame_stack

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
class frame_stack;
```

按后进先出的顺序保存一组组临时数据。每组可以包含不同类型的值、长度在运行时确定的数组，以及可以继续压入数据的子栈。

## 成员函数

|  |  |
| --- | --- |
| [`(构造函数)`](frame_stack/constructor.md) | 构造一个栈 |
| [`push`](frame_stack/push.md) | 加入一组数据 |
| [`top`](frame_stack/top.md) | 访问顶部数据 |
| [`pop`](frame_stack/pop.md) | 移除顶部数据 |
| [`empty`](frame_stack/empty.md) | 判断栈是否为空 |
| [`clear`](frame_stack/clear.md) | 移除全部数据 |
| [`size`](frame_stack/size.md) | 取得已用字节数 |
| [`capacity`](frame_stack/capacity.md) | 取得容量 |
| [`reserve`](frame_stack/reserve.md) | 调整容量 |
| [`data`](frame_stack/data.md) | 访问字节数据 |

## 非成员函数

|  |  |
| --- | --- |
| [`swap`](frame_stack/swap.md) | 交换两个栈的内容 |

## 注意

每组数据称为一帧。访问时须提供与所访问数据一致的类型序列。元素须可平凡复制，对齐要求不超过 [`max_alignment`](max_alignment.md)。

每帧按动态数组、固定元素、可选子栈的顺序组成。子栈最多一个，必须位于帧尾，通过 [`substack()`](substack.md) 创建。

同一个栈的多个帧可以各自包含子栈；子栈内部只能包含固定字段和动态数组组成的普通帧。

增加容量可能使已有 view、引用和指针失效。含栈顶子栈的访问形态另有可重新定位的保证，见 [`top`](frame_stack/top.md)；已经取得的元素引用和数组 span 不享有这项保证。移除一帧后，不得继续访问其中的数据。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    stack.push(3, 5);
    auto [first, second] = stack.top<int, int>();
    std::println("两个值之和: {}", first + second);
    stack.pop<int, int>();
    std::println("弹出后为空: {}", stack.empty());
}
```

输出

```text
两个值之和: 8
弹出后为空: true
```

## 参阅

|  |  |
| --- | --- |
| [`frame_view`](frame_view.md) | 一帧数据的访问对象 |
| [`dynamic_array`](dynamic_array.md) | 动态数组初始化参数的构造工具，是一个可调用常量 |
| [`substack`](substack.md) | 空子栈的初始化参数 |
