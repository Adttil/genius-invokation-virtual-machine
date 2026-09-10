[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_view](../definition_source_view.md) / **dependencies_by_tag**

# givm::definition_source_view::dependencies_by_tag

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDependencyCategory>
std::vector<std::string_view> dependencies_by_tag() const;
```

取得按标签筛选的一组定义依赖，例如需要从全部非料理治疗牌中选择一张时所用的筛选条件。

## 模板参数

|  |  |
| --- | --- |
| `TDependencyCategory` | 要查询的依赖类别 |

## 返回值

声明的字符串序列；源未声明对应成员时返回空序列。返回的容器拥有字符串视图，不拥有字符存储。

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
    std::println("卡牌筛选条件: {}", view.dependencies_by_tag<givm::card_definition>().front());
}
```

输出

```text
卡牌筛选条件: 治疗 & !料理
```
