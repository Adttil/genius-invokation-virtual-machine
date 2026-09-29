[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **resolve_id**

# givm::definition_compile_context::resolve_id

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<class TCategory>
definition_id<TCategory> resolve_id(std::string_view name) const;
```

按名称取得本定义所依赖的另一项定义，例如一张卡牌生成的支援定义。

## 模板参数

|  |  |
| --- | --- |
| `TCategory` | 依赖的定义类别 |

## 参数

|  |  |
| --- | --- |
| `name` | 本源在对应类别的 `*_dependencies()` 中声明的名称 |

## 返回值

已编译选择范围中该定义的 ID。名称未在本源的对应依赖声明中出现，或该定义不存在时，返回无效 ID，并向本次编译记录 [`definition_resolution_error`](../definition_resolution_error.md)。最终 [`compile`](../compile.md) 返回失败诊断，不抛出验证异常。

## 注意

失败后源的 `compile` 仍可继续执行，以收集其他独立错误；不要用无效 ID 索引元数据或访问定义。

## 示例

```cpp
#include <utility>
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
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0,
        givm::genshin_impact::shield_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(card, support)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
}
```

输出

```text
已找到依赖的支援: true
```
