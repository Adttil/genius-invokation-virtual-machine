[givm](../../reference.md) / [通用工具](../utils.md) / **substack**

# givm::substack

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr auto substack() noexcept;
```

构造空子栈的初始化参数，用于在一帧末尾附带另一个可继续压入数据的栈。

## 返回值

供 [`frame_stack::push`](frame_stack/push.md) 使用的初始化参数。对应帧元素通过 [`substack_t`](substack_t.md) 描述。

## 注意

初始化参数必须是 `push` 的最后一个实参，每帧最多提供一次。创建空子栈不要求同时提供数组或固定元素。

子栈内部只能压入普通帧，不能向子栈的 `push` 传入此初始化参数。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto [child] = stack.push(givm::substack());
    child.push(7);
    std::println("子栈顶部: {}", child.top<int>().get<0>());
    child.pop<int>();
    std::println("子栈为空: {}", child.empty());
    std::println("父栈为空: {}", stack.empty());
}
```

输出

```text
子栈顶部: 7
子栈为空: true
父栈为空: false
```
