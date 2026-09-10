[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **capacity**

# givm::frame_stack::capacity

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr size_t capacity() const noexcept;
```

取得栈不重新分配空间时能够容纳的数据量。

## 返回值

容量，单位为字节。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{ 64 };
    std::println("容量: {}", stack.capacity());
    std::println("已有数据未超过容量: {}", stack.size() <= stack.capacity());
}
```

输出

```text
容量: 64
已有数据未超过容量: true
```
