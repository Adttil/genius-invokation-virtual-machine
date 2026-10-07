[givm](../../reference.md) / [牌桌](../table.md) / **hand_card_view**

# givm::hand_card_view

定义于头文件 `<givm/table.hpp>`

```cpp
class hand_card_view;
```

玩家手中一张卡牌的只读视图。它代表这张正在参与对局的牌，可以拥有自己的附带状态。

## 静态成员

| | |
| --- | --- |
| [`category`](hand_card_view/category.md) | 此视图的实体类别 |

## 成员函数

|  |  |
| --- | --- |
| [`table`](hand_card_view/table.md) | 取得所属牌桌 |
| [`is_valid`](hand_card_view/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](hand_card_view/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](hand_card_view/size.md) | 取得单实体范围的元素数 |
| [`begin`](hand_card_view/begin.md) | 取得单实体范围的起点 |
| [`end`](hand_card_view/end.md) | 取得单实体范围的终点 |
| [`player`](hand_card_view/player.md) | 取得所属玩家 |
| [`id`](hand_card_view/id.md) | 取得实体 ID |
| [`definition_id`](hand_card_view/definition_id.md) | 取得实体的定义 ID |
| [`state`](hand_card_view/state.md) | 访问实体状态 |
| [`statuses`](hand_card_view/statuses.md) | 遍历卡牌状态 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个手牌。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。

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
    std::println("采用已加载的定义: {}", view.definition_id() == definition);
}
```

输出

```text
采用已加载的定义: true
```

## 参阅

|  |  |
| --- | --- |
| [`card_state`](card_state.md) | 该实体的状态 |
