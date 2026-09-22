[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **has_any_tag**

# givm::definition_library::has_any_tag

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TDefinitionType>
bool has_any_tag(definition_id<TDefinitionType> id, std::span<const tag_id> tags) const;
```

检查一个定义是否具有所列标签中的任意一个标签。

## 模板参数

|  |  |
| --- | --- |
| `TDefinitionType` | 定义类别 |

## 参数

|  |  |
| --- | --- |
| `id` | 本定义库中的有效定义 ID |
| `tags` | 本定义库中的有效标签 ID 范围 |

## 返回值

满足条件时返回 `true`，否则返回 `false`。空标签范围返回 `false`。

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
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
    const auto card = ids.get_id<givm::card_definition>("恢复药剂");
    const std::array tags{ ids.get_tag_id("治疗") };
    std::println("符合分类条件: {}", library.has_any_tag(card, tags));
}
```

输出

```text
符合分类条件: true
```
