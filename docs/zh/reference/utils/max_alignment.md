[givm](../../reference.md) / [通用工具](../utils.md) / **max_alignment**

# givm::max_alignment

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
inline constexpr size_t max_alignment = alignof(std::max_align_t);
```

栈支持的元素对齐上限，用于判断一种类型是否适合放入栈。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    std::println("int 的对齐满足要求: {}", alignof(int) <= givm::max_alignment);
}
```

输出

```text
int 的对齐满足要求: true
```
