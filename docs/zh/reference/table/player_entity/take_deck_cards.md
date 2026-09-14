[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **take_deck_cards**

# givm::player_entity::take_deck_cards

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr std::vector<card_data> take_deck_cards(
    std::span<const size_t> descending_indices
) const requires is_mutable;
```

从牌库取出指定位置上的多张牌，保留每张牌的状态。

## 参数

|  |  |
| --- | --- |
| `descending_indices` | 严格递减、不重复的牌库位置；每项都小于调用前的 deck_card_count() |

## 返回值

按传入位置顺序排列的 [`card_data`](../card_data.md) 列表。

## 注意

未被取出卡牌的相对顺序保持不变。原实体变为无效；返回值用于同一牌桌内的卡牌搬运。

## 示例

```cpp
#include <array>
#include <cstddef>
#include <print>
#include <string_view>
#include <tuple>
#include <utility>

#include <givm/givm.hpp>

template<class Category>
struct example_source
{
    using definition_category = Category;
    struct definition_type {};

    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    givm::definition_source_library sources{};
    const example_source<givm::card_definition> card_source{};
    sources.add(card_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    for(int i = 0; i < 3; ++i)
    {
        player.add_deck_card(definition, {});
    }
    const std::array<std::size_t, 2> selected{ 2, 0 };
    auto cards = player.take_deck_cards(selected);
    std::println("取出张数: {}", cards.size());
    std::println("牌库剩余张数: {}", player.deck_card_count());
    for(auto& card : cards)
    {
        player.add_hand_card(std::move(card));
    }
    std::println("移入手牌张数: {}", player.hand_card_count());
}
```

输出

```text
取出张数: 2
牌库剩余张数: 1
移入手牌张数: 2
```
