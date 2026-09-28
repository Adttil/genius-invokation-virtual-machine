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
