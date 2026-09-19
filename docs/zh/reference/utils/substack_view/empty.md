[givm](../../../reference.md) / [通用工具](../../utils.md) / [子栈视图](../substack_view.md) / **empty**

# 子栈视图的 empty

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr bool empty() const noexcept;
```

判断子栈是否没有内部帧。

## 返回值

没有内部帧时返回 `true`，否则返回 `false`。空子栈仍是父帧的一部分。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto [child] = stack.push(givm::substack());
    std::println("初始为空: {}", child.empty());
    child.push(3);
    std::println("压入后为空: {}", child.empty());
    child.pop<int>();
    std::println("弹出后为空: {}", child.empty());
}
```

输出

```text
初始为空: true
压入后为空: false
弹出后为空: true
```
