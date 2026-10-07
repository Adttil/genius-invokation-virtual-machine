[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **matches_tags**

# givm::definition_library::matches_tags

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<definition_category TDefinitionType>
bool matches_tags(
    definition_id<TDefinitionType> id,
    std::span<const tag_id> required_tags,
    std::span<const tag_id> excluded_tags = {}
) const;
```

检查定义是否同时满足所需标签和排除标签的条件。

## 模板参数

|  |  |
| --- | --- |
| `TDefinitionType` | 定义类别 |

## 参数

|  |  |
| --- | --- |
| `id` | 本定义库中的有效定义 ID |
| `required_tags` | 必须全部具有的标签 |
| `excluded_tags` | 必须全部不具有的标签 |

## 返回值

全部条件满足时返回 `true`，否则返回 `false`。两个标签范围均为空时返回 `true`。

## 注意

所有标签 ID 须属于本定义库且有效。

## 示例

```cpp
#include <utility>
#include <array>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct card_source
{
    static constexpr auto category = givm::definition_category::card;

    std::string_view name() const { return "恢复药剂"; }
    auto tags() const { return std::array<std::string_view, 1>{ "治疗" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
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
    const auto card = ids.get_id<givm::definition_category::card>("恢复药剂");
    const std::array healing{ ids.get_tag_id("治疗") };
    std::println("要求治疗标签: {}", library.matches_tags(card, healing));
    std::println("排除治疗标签: {}", library.matches_tags(card, {}, healing));
}
```

输出

```text
要求治疗标签: true
排除治疗标签: false
```
