[givm](../../reference.md) / [定义](../definition.md) / **definition_source_view**

# givm::definition_source_view

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TCategory>
class definition_source_view;
```

一份定义源的只读视图。不同的卡牌或角色可以用各自的 C++ 类型编写，源库通过这个视图统一查看它们的名称、分类和依赖。

## 模板参数

|  |  |
| --- | --- |
| `TCategory` | 定义类别，见 [`definition_types`](definition_types.md) |

## 成员函数

|  |  |
| --- | --- |
| [(构造函数)](definition_source_view/constructor.md) | 从定义源构造视图 |
| [`name`](definition_source_view/name.md) | 取得定义名称 |
| [`tags`](definition_source_view/tags.md) | 取得分类标签 |
| [`dependencies`](definition_source_view/dependencies.md) | 取得按名称声明的依赖 |
| [`tag_dependencies`](definition_source_view/tag_dependencies.md) | 取得将直接查询的标签 |
| [`dependencies_by_tag`](definition_source_view/dependencies_by_tag.md) | 取得按标签筛选的依赖 |

## 注意

视图不拥有源对象。源对象以及它返回的名称、标签和依赖字符串的字符存储必须保持有效。

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view name() const { return "召唤卡"; }
    auto tags() const { return std::array<std::string_view, 1>{ "召唤" }; }
    auto support_dependencies() const
    { return std::array<std::string_view, 1>{ "协助者" }; }
    auto tag_dependencies() const
    { return std::array<std::string_view, 1>{ "治疗" }; }
    auto card_dependencies_by_tag() const
    { return std::array<std::string_view, 1>{ "治疗 & !料理" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    const givm::definition_source_view<givm::card_definition> view{ source };
    std::println("定义源名称: {}", view.name());
    std::println("依赖的支援: {}", view.dependencies<givm::support_view>().front());
}
```

输出

```text
定义源名称: 召唤卡
依赖的支援: 协助者
```
