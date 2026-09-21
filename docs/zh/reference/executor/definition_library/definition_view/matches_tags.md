[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **matches_tags**

# givm::definition_library::definition_view::matches_tags

定义于头文件 `<givm/executor.hpp>`

```cpp
bool matches_tags(
    std::span<const tag_id> required_tags,
    std::span<const tag_id> excluded_tags = {}
) const;
```

检查该定义是否符合必需标签与排除标签共同描述的条件。

## 参数

|  |  |
| --- | --- |
| `required_tags` | 必须全部具有的标签 |
| `excluded_tags` | 必须全部不具有的标签 |

## 返回值

全部条件满足时返回 `true`，否则返回 `false`。两个范围均为空时返回 `true`。

## 注意

标签 ID 须属于配套定义库且有效。

## 示例

```cpp
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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }, givm::compile_mode::normal
    );
    const auto card = ids.get_id<givm::card_definition>("恢复药剂");
    const std::array healing{ ids.get_tag_id("治疗") };
    std::println("要求治疗标签: {}", library[card].matches_tags(healing));
    std::println("排除治疗标签: {}", library[card].matches_tags({}, healing));
}
```

输出

```text
要求治疗标签: true
排除治疗标签: false
```
