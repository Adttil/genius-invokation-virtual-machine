[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **empty**

# givm::frame_stack::empty

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr bool empty() const noexcept;
```

判断栈是否没有数据。

## 返回值

空栈返回 `true`，否则返回 `false`。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    std::println("初始为空: {}", stack.empty());
    stack.push(7);
    std::println("加入数据后为空: {}", stack.empty());
}
```

输出

```text
初始为空: true
加入数据后为空: false
```
