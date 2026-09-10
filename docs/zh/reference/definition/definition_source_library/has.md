[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **has**

# givm::definition_source_library::has

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDefinitionType>
bool has(std::string_view name) const;
```

检查某类定义源中是否已登记指定名称。

## 模板参数

|  |  |
| --- | --- |
| `TDefinitionType` | 定义类别，见 [`definition_types`](../definition_types.md) |

## 参数

|  |  |
| --- | --- |
| `name` | 定义源名称 |

## 返回值

已登记时返回 `true`，否则返回 `false`。

## 示例

```cpp
#include <print>
#include <string_view>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view source_name;

    std::string_view name() const { return source_name; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source potion{ "恢复药剂" };
    givm::definition_source_library sources{};
    sources.add(potion);
    std::println("包含恢复药剂: {}", sources.has<givm::card_definition>("恢复药剂"));
    std::println("包含恢复料理: {}", sources.has<givm::card_definition>("恢复料理"));
}
```

输出

```text
包含恢复药剂: true
包含恢复料理: false
```
