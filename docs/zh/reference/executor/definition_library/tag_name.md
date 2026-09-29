[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **tag_name**

# givm::definition_library::tag_name

定义于头文件 `<givm/runtime.hpp>`

```cpp
std::string_view tag_name(tag_id id) const;
```

取得标签的名称，便于展示定义的分类。

## 参数

|  |  |
| --- | --- |
| `id` | 本定义库中的有效标签 ID |

## 返回值

对应的标签名称。

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
    std::println("标签名称: {}", library.tag_name(ids.get_tag_id("治疗")));
}
```

输出

```text
标签名称: 治疗
```
