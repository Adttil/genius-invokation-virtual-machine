[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_view](../deck_card_view.md) / **player**

# givm::deck_card_view::player

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr player_view player() const;
```

取得这个牌库卡牌所属的玩家。

## 返回值

[`player_view`](../player_view.md)。

## 示例

```cpp
#include <utility>
#include <cstdint>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct example_source
{
    static constexpr auto category = givm::definition_category::card;
    struct definition_type {};
    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    example_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    const auto definition = ids.get_id<givm::definition_category::card>("示例");
    givm::table table{};
    load_deck(table, library, givm::linked_deck{ .cards = { definition } }, {});
    const givm::deck_card_view view = table[givm::deck_card_id{ givm::player_id{ 0 }, 0 }];
    std::println("所属玩家: {}", view.player().id().index());
}
```

输出

```text
所属玩家: 0
```
