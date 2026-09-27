[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_view](../hand_card_view.md) / **definition_id**

# givm::hand_card_view::definition_id

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto definition_id() const;
```

取得该实体采用的定义 ID。

## 返回值

该实体的 [`definition_id`](../definition_id.md)。

## 注意

实体须曾存在于此处，且未转移、未清理；删除后仍可读取本项信息。访问对象与 ID 的保存期限见[实体的身份与访问](../entity_access.md)。


## 示例

```cpp
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
    const auto [library, ids] = compile(
        sources, basics,
        std::tuple{ givm::draw_cards{ .positions = draw_positions }, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{}, givm::compile_mode::normal);
    const auto definition = ids.get_id<givm::card_definition>("示例");
    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    load_deck(table, library, givm::linked_deck{ .cards = { definition } }, {});

    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.start(library, table);
    execution.step(library, table, random);
    const givm::hand_card_view view = table[givm::hand_card_id{ givm::player_id{ 0 }, 0 }];
    std::println("采用已加载的定义: {}", view.definition_id() == definition);
}
```

输出

```text
采用已加载的定义: true
```
