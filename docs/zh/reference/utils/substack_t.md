[givm](../../reference.md) / [通用工具](../utils.md) / **substack_t**

# givm::substack_t

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
struct substack_t;
```

表示帧末尾包含一个子栈。子栈可以继续压入和弹出由固定字段、动态数组组成的普通帧，内部帧不能包含 `substack_t`。

## 用法

创建帧时通过 [`substack()`](substack.md) 初始化为空子栈。调用 `top`、`pop` 或编写 [`frame`](frame.md) 描述时，用 `substack_t` 表示对应元素。

每帧最多一个子栈，必须位于动态数组和固定元素之后。子栈没有内部帧时仍然作为父帧的一个元素存在；只有弹出整个父帧才会连同子栈一起移除。

同一个 `frame_stack` 中可以有多个帧各自包含一个子栈。只有最上方帧的子栈可以增删内部帧。

`substack_t` 是单个子栈元素的标记，不能作为帧中数组的元素类型。

通过帧 view 访问该元素得到[子栈视图](substack_view.md)，其具体类型不作保证。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    stack.push(3, givm::substack());
    auto frame = stack.top<int, givm::substack_t>();
    auto [value, child] = frame;
    std::println("父帧的值: {}", value);
    std::println("子栈为空: {}", child.empty());
    stack.pop<int, givm::substack_t>();
    std::println("父栈为空: {}", stack.empty());
}
```

输出

```text
父帧的值: 3
子栈为空: true
父栈为空: true
```
