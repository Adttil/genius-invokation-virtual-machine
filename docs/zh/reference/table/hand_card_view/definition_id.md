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

实体必须有效。


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
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::draw_cards{ .count = 1 }, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{});
    const auto definition = ids.get_id<givm::card_definition>("示例");
    givm::card_table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .cards = { definition } });

    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(library, table, random);
    const givm::hand_card_view view = table[givm::hand_card_id{ givm::player_id{ 0 }, 0 }];
    std::println("采用已加载的定义: {}", view.definition_id() == definition);
}
```

输出

```text
采用已加载的定义: true
```
