[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **has_any_tag**

# givm::definition_library::definition_view::has_any_tag

定义于头文件 `<givm/executor.hpp>`

```cpp
bool has_any_tag(std::span<const tag_id> tags) const;
```

检查该定义是否具有给定标签中的任意一个标签。

## 参数

|  |  |
| --- | --- |
| `tags` | 配套定义库中的有效标签 ID 范围 |

## 返回值

满足条件时返回 `true`，否则返回 `false`。空范围返回 `false`。

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
    using definition_category = givm::card_definition;

    std::string_view name() const { return "恢复药剂"; }
    auto tags() const { return std::array<std::string_view, 1>{ "治疗" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(source)) return 1;
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
    const auto card = ids.get_id<givm::card_definition>("恢复药剂");
    const std::array tags{ ids.get_tag_id("治疗") };
    std::println("符合分类条件: {}", library[card].has_any_tag(tags));
}
```

输出

```text
符合分类条件: true
```
