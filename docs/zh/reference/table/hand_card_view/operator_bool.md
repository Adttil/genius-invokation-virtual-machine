[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_view](../hand_card_view.md) / **operator bool**

# givm::hand_card_view::operator bool

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr explicit operator bool() const;
```

判断这个手牌是否仍然有效。

## 返回值

实体尚未移除时为 `true`，已经移除时为 `false`。

## 注意

访问对象本身必须仍可安全访问；本函数不能用来检查已经悬空的对象。

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
        std::tuple{ givm::draw_cards{ .position = 0, .count = 1 }, givm::settle{}, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    const auto definition = ids.get_id<givm::definition_category::card>("示例");
    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    load_deck(table, library, givm::linked_deck{ .cards = { definition } }, {});

    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    const givm::hand_card_view view = table[givm::hand_card_id{ givm::player_id{ 0 }, 0 }];
    if(view)
    {
        std::println("有效实体的定义: {}", library[view.definition_id()].name());
    }
}
```

输出

```text
有效实体的定义: 示例
```
