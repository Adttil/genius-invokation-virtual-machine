[givm](../../reference.md) / [通用工具](../utils.md) / **子栈视图**

# 子栈视图

定义于头文件 `<givm/utils/stack.hpp>`

通过帧 view 访问 [`substack_t`](substack_t.md) 元素时取得的访问对象。它让一组临时数据可以继续拥有自己的栈，父帧的其他元素不需要一同压入或弹出。

具体视图类型不作保证，使用 `auto` 或结构化绑定接收。

## 成员函数

|  |  |
| --- | --- |
| [`push`](substack_view/push.md) | 在子栈顶部加入一帧 |
| [`top`](substack_view/top.md) | 访问子栈顶部的一帧、后缀或多帧 |
| [`pop`](substack_view/pop.md) | 移除子栈顶部的帧 |
| [`empty`](substack_view/empty.md) | 判断子栈是否没有内部帧 |
| [`size`](substack_view/size.md) | 取得子栈内部帧占用的字节数 |

## 注意

子栈只能在其所属帧位于 `frame_stack` 栈顶时增删内部帧。通过只读栈取得的子栈不能修改内容；通过多帧访问取得的较早帧中的子栈不能增删内部帧，但仍可按 const 属性修改已有内容。

子栈内部只允许由固定字段和动态数组组成的普通帧，不能含有 `substack_t`。同一个 `frame_stack` 中的多个帧仍可各自包含子栈。

弹出最后一个内部帧后，子栈仍然存在并可以再次压入数据。弹出父帧会一并移除子栈及全部内部帧。

子栈增删可能导致存储扩容。符合 [`frame_stack::top`](frame_stack/top.md) 规则的帧 view 可以重新取得元素，已经取得的字段引用或 span 仍可能失效。父帧被覆盖或弹出，以及所属栈移动、赋值、交换后，应重新取得相关 view。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto parent = stack.push(3, givm::substack());
    auto child = parent.get<1>();
    child.push(5);
    child.push(8);
    auto [first, second] = child.top<givm::frame<int>, givm::frame<int>>();
    std::println("较早子帧: {}", first.get<0>());
    std::println("顶部子帧: {}", second.get<0>());
    std::println("父帧固定值: {}", parent.get<0>());
    child.pop<givm::frame<int>, givm::frame<int>>();
    std::println("子栈为空: {}", child.empty());
}
```

输出

```text
较早子帧: 5
顶部子帧: 8
父帧固定值: 3
子栈为空: true
```
