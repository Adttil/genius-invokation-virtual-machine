[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **insert_deck_card**

# givm::player_entity::insert_deck_card

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr deck_card_entity<TStorage> insert_deck_card(
    size_t index, definition_id<card_definition> definition_id,
    const card_state& state
) const requires is_mutable;

constexpr deck_card_entity<TStorage> insert_deck_card(
    size_t index, card_data data
) const requires is_mutable;
```

在牌库的指定位置插入一张新卡牌或已取出的卡牌。

## 参数

|  |  |
| --- | --- |
| `index` | 插入前的牌库顺序位置，范围为 0 到 deck_card_count()，包含两端 |
| [`definition_id`](../definition_id.md) | 新卡牌的定义 ID |
| `state` | 新卡牌的初始状态 |
| `data` | 从同一张牌桌取出的 [`card_data`](../card_data.md) |

## 返回值

新插入的 [`deck_card_entity`](../deck_card_entity.md)。

## 注意

位置 0 为牌库底；在 `deck_card_count()` 处插入即放到牌库顶。`index` 是顺序位置，不是 `deck_card_id::index`。

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>

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
    const auto first_id = player.add_deck_card(definition, {}).id();
    player.insert_deck_card(0, definition, {});
    std::println("牌库张数: {}", player.deck_card_count());
    std::println("原来的牌仍在牌库顶: {}", player.deck_cards<false>()[1].id() == first_id);
}
```

输出

```text
牌库张数: 2
原来的牌仍在牌库顶: true
```
