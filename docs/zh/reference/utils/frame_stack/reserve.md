[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **reserve**

# givm::frame_stack::reserve

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr void reserve(size_t new_capacity);
```

重新设置栈的容量，保留已有数据。

## 参数

|  |  |
| --- | --- |
| `new_capacity` | 新的字节容量，不得小于当前 `size()` |

## 返回值

（无）

## 注意

本操作会重新分配存储，即使新容量没有增大，普通帧 view、字段引用、指针和 span 也会失效。含栈顶子栈的可重新定位 view 见 [`top`](top.md)；它们仍须重新取得原有字段引用或 span。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    stack.push(7);
    stack.reserve(128);
    std::println("新容量: {}", stack.capacity());
    std::println("保留的值: {}", stack.top<int>().get<0>());
}
```

输出

```text
新容量: 128
保留的值: 7
```
