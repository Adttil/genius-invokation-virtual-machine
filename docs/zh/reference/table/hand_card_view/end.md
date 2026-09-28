[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_view](../hand_card_view.md) / **end**

# givm::hand_card_view::end

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto end(this const auto& self);
```

取得这个手牌的单实体范围终点。

## 参数

|  |  |
| --- | --- |
| `self` | 当前实体的只读视图 |

## 返回值

单实体范围的尾后迭代器。实体有效时范围包含一个元素，否则与 `begin()` 相等。

## 示例

```cpp
#include <utility>
#include <array>
#include <cstddef>
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
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(source)) return 1;
    constexpr std::array<std::size_t, 1> draw_positions{ 0 };
    auto library_result = compile(
        sources, basics,
        std::tuple{ givm::draw_cards{ .positions = draw_positions }, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    const auto definition = ids.get_id<givm::card_definition>("示例");
    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    load_deck(table, library, givm::linked_deck{ .cards = { definition } }, {});

    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.start(library, table);
    execution.step(library, table, random);
    const givm::hand_card_view view = table[givm::hand_card_id{ givm::player_id{ 0 }, 0 }];
    std::println("范围长度: {}", std::ranges::distance(view.begin(), view.end()));
}
```

输出

```text
范围长度: 1
```
