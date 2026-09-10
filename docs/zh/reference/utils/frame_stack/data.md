[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **data**

# givm::frame_stack::data

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr unsigned char* data() noexcept;
constexpr const unsigned char* data() const noexcept;
```

访问栈的数据区域，供需要按字节读取数据的接口使用。

## 返回值

指向数据区域起始位置的指针；尚未分配空间的空栈返回空指针。

## 注意

有效数据长度由 `size()` 给出。重新分配空间会使指针失效；不要把原始字节视为跨平台的持久化格式。

## 示例

```cpp
#include <print>
#include <span>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{ 64 };
    stack.push(7);
    std::span<const unsigned char> bytes{ stack.data(), stack.size() };
    std::println("可读取的数据非空: {}", !bytes.empty());
}
```

输出

```text
可读取的数据非空: true
```
