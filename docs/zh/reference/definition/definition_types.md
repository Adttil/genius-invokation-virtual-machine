[givm](../../reference.md) / [定义](../definition.md) / **definition_types**

# givm::definition_types

定义于头文件 `<givm/definition.hpp>`

```cpp
using definition_types = type_list<
    card_definition,
    status_definition,
    support_view,
    summon_view,
    combat_status_view,
    character_view,
    skill_view,
    attachment_view,
    history_summary_definition
>;
```

定义系统支持的全部定义类别，包括场上实体与历史摘要。编写通用的定义管理操作，或按类别填写对局选择时，可以通过这份类型列表枚举与定位各类别。

## 注意

成员操作继承自 [`type_list`](../utils/type_list.md)。卡牌与卡牌状态各自包含手牌和牌堆两种实体形态，其他实体类别对应各自的只读 view。[`history_summary_definition`](history_summary.md) 不对应场上实体，其状态由摘要更新函数访问，配套牌桌提供只读字段访问。

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

    std::string_view source_name;

    std::string_view name() const { return source_name; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source potion{ "恢复药剂" };
    const card_source food{ "恢复料理" };
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0,
        givm::genshin_impact::shield_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(potion, food)) return 1;
    const std::array<std::string_view, 1> names{ "恢复药剂" };
    givm::definition_selection selection{};
    selection[givm::definition_types::index_of<givm::card_definition>()] = names;
    auto library_result = compile(sources, basics, selection, std::tuple{},
        std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } },
        givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    std::println("包含恢复药剂: {}", ids.has<givm::card_definition>("恢复药剂"));
    std::println("包含恢复料理: {}", ids.has<givm::card_definition>("恢复料理"));
}
```

输出

```text
包含恢复药剂: true
包含恢复料理: false
```
