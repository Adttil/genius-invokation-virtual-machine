[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **size**

# givm::frame_stack::size

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr size_t size() const noexcept;
```

取得栈中数据占用的字节数。

## 返回值

已用字节数，包含存放数据所需的对齐空间。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    std::println("初始字节数: {}", stack.size());
    stack.push(7);
    std::println("加入数据后占用空间: {}", stack.size() > 0);
}
```

输出

```text
初始字节数: 0
加入数据后占用空间: true
```
