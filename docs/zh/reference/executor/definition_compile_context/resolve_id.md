[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **resolve_id**

# givm::definition_compile_context::resolve_id

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<definition_category TCategory>
optional_definition_id<TCategory> resolve_id(std::string_view name) const;
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

已编译选择范围中该定义的可空 ID。名称未在本源的对应依赖声明中出现，或该定义不存在时，返回空值，并向本次编译记录 [`definition_resolution_error`](../definition_resolution_error.md)。最终 [`compile`](../compile.md) 返回失败诊断，不抛出验证异常。

## 注意

失败后源的 `compile` 仍可继续执行，以收集其他独立错误。使用 `if(id)` 判断是否取得结果；只有取得的强类型 `id.get()` 可用于索引元数据或访问定义。

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
    static constexpr auto category = givm::definition_category::support;
    std::string_view name() const { return "协助者"; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

struct card_source
{
    static constexpr auto category = givm::definition_category::card;
    std::string_view name() const { return "召唤卡"; }
    auto support_dependencies() const
    { return std::array<std::string_view, 1>{ "协助者" }; }

    givm::optional_definition_id<givm::definition_category::support> compile(givm::definition_compile_context& context) const
    {
        const auto support = context.resolve_id<givm::definition_category::support>("协助者");
        std::println("已找到依赖的支援: {}", bool(support));
        return support;
    }
};

int main()
{
    const card_source card{};
    const support_source support{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(card, support)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{}, givm::settle{} }, givm::compile_mode::normal
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
