[givm](../../reference.md) / [定义](../definition.md) / **definition_compile_context**

# givm::definition_compile_context

定义于头文件 `<givm/definition.hpp>`

```cpp
class definition_compile_context;
```

编写实体定义时使用的编译环境。它让卡牌或角色找到事先声明的其他定义与标签，并登记响应事件时要执行的效果。

## 成员函数

|  |  |
| --- | --- |
| [`resolve_id`](definition_compile_context/resolve_id.md) | 按名称取得已声明依赖的定义 ID |
| [`resolve_tag`](definition_compile_context/resolve_tag.md) | 取得已声明使用的标签 ID |
| [`resolve_ids_by_tag`](definition_compile_context/resolve_ids_by_tag.md) | 取得已声明筛选条件匹配的定义 |
| [`add_program`](definition_compile_context/add_program.md) | 登记一段效果并取得入口 |

## 注意

由源库在调用定义源的 `compile` 时提供，只在本次编译调用中使用。依赖须通过[定义源协议](source_protocol.md)提前声明。

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct support_source
{
    using definition_category = givm::support_view;
    std::string_view name() const { return "协助者"; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

struct card_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "召唤卡"; }
    auto support_dependencies() const
    { return std::array<std::string_view, 1>{ "协助者" }; }

    givm::definition_id<givm::support_view> compile(givm::definition_compile_context& context) const
    {
        const auto support = context.resolve_id<givm::support_view>("协助者");
        std::println("已找到依赖的支援: {}", support.is_valid());
        return support;
    }
};

int main()
{
    const card_source card{};
    const support_source support{};
    givm::definition_source_library sources{};
    sources.add(card, support);
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ givm::start_round{} }
    );
}
```

输出

```text
已找到依赖的支援: true
```
