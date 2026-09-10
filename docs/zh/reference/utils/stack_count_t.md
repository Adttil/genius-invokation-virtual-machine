[givm](../../reference.md) / [通用工具](../utils.md) / **stack_count_t**

# givm::stack_count_t

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
using stack_count_t = size_t;
```

栈中动态数组的元素数量类型。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    const givm::stack_count_t count = 3;
    givm::frame_stack stack{};
    auto [values] = stack.push(givm::dynamic_array<int>(count));
    std::println("预留的元素数量: {}", values.size());
}
```

输出

```text
预留的元素数量: 3
```
