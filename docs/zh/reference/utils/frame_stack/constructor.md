[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **frame_stack**

# givm::frame_stack::frame_stack

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr frame_stack() noexcept;
constexpr explicit frame_stack(size_t capacity);
constexpr frame_stack(const frame_stack& other);
constexpr frame_stack(frame_stack&& other) noexcept;
```

构造空栈、具有给定容量的空栈，或从另一栈复制、移动数据。

## 参数

|  |  |
| --- | --- |
| `capacity` | 初始容量，单位为字节 |
| `other` | 要复制或移动的栈 |

## 返回值

（无）

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack original{ 64 };
    original.push(7);
    givm::frame_stack copy{ original };
    copy.top<int>().get<0>() = 9;
    std::println("原栈的值: {}", original.top<int>().get<0>());
    std::println("副本的值: {}", copy.top<int>().get<0>());
}
```

输出

```text
原栈的值: 7
副本的值: 9
```
