[givm](../../../reference.md) / [定义](../../definition.md) / [definition_library](../definition_library.md) / **definition_count**

# givm::definition_library::definition_count

定义于头文件 `<givm/definition.hpp>`

```cpp
static constexpr size_t definition_count = definition_types::size();
```

定义系统支持的类别数量，不是某个库中已登记定义的数量。可用于为每个类别准备统计或配置项。

## 示例

```cpp
#include <array>
#include <cstddef>
#include <print>

#include <givm/givm.hpp>

int main()
{
    std::array<std::size_t, givm::definition_library::definition_count> selected_counts{};
    selected_counts[givm::definition_types::index_of<givm::card_definition>()] = 30;
    std::println("类别数量: {}", selected_counts.size());
    std::println("计划选择的卡牌数: {}", selected_counts[givm::definition_types::index_of<givm::card_definition>()]);
}
```

输出

```text
类别数量: 8
计划选择的卡牌数: 30
```
