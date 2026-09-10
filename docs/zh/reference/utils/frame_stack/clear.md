[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **clear**

# givm::frame_stack::clear

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr void clear() noexcept;
```

移除栈中的全部数据，保留容量以供后续使用。

## 返回值

（无）

## 注意

调用后，已有数据的 view、引用和指针不可再用于访问被移除的数据。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{ 64 };
    stack.push(7);
    stack.clear();
    std::println("清空后为空: {}", stack.empty());
    std::println("容量保留: {}", stack.capacity() == 64);
}
```

输出

```text
清空后为空: true
容量保留: true
```
