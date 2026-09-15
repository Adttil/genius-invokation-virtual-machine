[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_view](../hand_card_view.md) / **player**

# givm::hand_card_view::player

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr player_view player() const;
```

取得这个手牌所属的玩家。

## 返回值

[`player_view`](../player_view.md)。


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
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::draw_cards{ .count = 1 }, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{}, givm::compile_mode::normal);
    const auto definition = ids.get_id<givm::card_definition>("示例");
    givm::table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .cards = { definition } });

    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    const givm::hand_card_view view = table[givm::hand_card_id{ givm::player_id{ 0 }, 0 }];
    std::println("所属玩家: {}", view.player().id().index);
}
```

输出

```text
所属玩家: 0
```
