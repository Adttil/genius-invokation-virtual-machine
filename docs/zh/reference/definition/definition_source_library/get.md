[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **get**

# givm::definition_source_library::get

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDefinitionType>
definition_source_view<TDefinitionType> get(std::string_view name) const;
```

按名称查看一份定义源的说明，例如它所属的标签及所需的其他定义。

## 模板参数

|  |  |
| --- | --- |
| `TDefinitionType` | 定义类别，见 [`definition_types`](../definition_types.md) |

## 参数

|  |  |
| --- | --- |
| `name` | 已登记的定义源名称 |

## 返回值

该源的 [`definition_source_view`](../definition_source_view.md)。

## 异常

|  |  |
| --- | --- |
| `std::out_of_range` | 名称未在指定类别中登记 |

## 注意

返回的 view 不拥有源对象；源对象及其字符串必须保持有效。

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view name() const { return "恢复药剂"; }
    auto tags() const { return std::array<std::string_view, 1>{ "治疗" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto view = sources.get<givm::card_definition>("恢复药剂");
    std::println("定义源名称: {}", view.name());
    std::println("首个标签: {}", view.tags().front());
}
```

输出

```text
定义源名称: 恢复药剂
首个标签: 治疗
```
