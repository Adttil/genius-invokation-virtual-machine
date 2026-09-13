[givm](../../reference.md) / [执行](../executor.md) / **selection_capacity**

# givm::selection_capacity

定义于头文件 `<givm/executor.hpp>`

```cpp
inline constexpr size_t selection_capacity = 64;
```

一次选择能够表示的位置数量。有效位置从零开始，小于此值。

## 示例

```cpp
#include <bitset>
#include <print>

#include <givm/givm.hpp>

int main()
{
    std::bitset<givm::selection_capacity> selection{};
    selection.set(givm::selection_capacity - 1);
    std::println("可表示的位置数量: {}", selection.size());
    std::println("已选择最后一个位置: {}", selection.test(givm::selection_capacity - 1));
}
```

输出

```text
可表示的位置数量: 64
已选择最后一个位置: true
```
