[givm](../../../reference.md) / [牌桌](../../table.md) / [player_view](../player_view.md) / **deck_card_definition**

# givm::player_view::deck_card_definition

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr definition_id<card_definition> deck_card_definition(size_t index) const;
```

取得牌库指定位置上的卡牌定义，以便查询该位置是什么牌。

## 参数

|  |  |
| --- | --- |
| `index` | 小于 deck_card_count() 的牌库顺序位置 |

## 返回值

该位置上的 [`definition_id<card_definition>`](../definition_id.md)。


## 示例

```cpp
#include <cstdint>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct example_source
{
    using definition_category = givm::card_definition;
    struct definition_type {};
    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    example_source source{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    const auto definition = ids.get_id<givm::card_definition>("示例");
    givm::table table{};
    load_deck(table, library, givm::linked_deck{ .cards = { definition } }, {});
    const givm::deck_card_view view = table[givm::deck_card_id{ givm::player_id{ 0 }, 0 }];
    std::println("牌库底部采用已加载的定义: {}", view.player().deck_card_definition(0) == definition);
}
```

输出

```text
牌库底部采用已加载的定义: true
```
