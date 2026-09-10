[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **take_top_deck_card**

# givm::player_entity::take_top_deck_card

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr card_data take_top_deck_card() const requires is_mutable;
```

从牌库顶取出一张牌，保留其状态以供移到其他位置。

## 返回值

取出的 [`card_data`](../card_data.md)，包含卡牌定义和原有状态。

## 注意

牌库必须非空。原牌库实体变为无效；返回的数据仅用于同一张牌桌内的卡牌搬运。该操作本身不广播抽牌事件。

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
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    player.add_deck_card(definition, {});
    player.add_hand_card(player.take_top_deck_card());
    std::println("牌库张数: {}", player.deck_card_count());
    std::println("手牌张数: {}", player.hand_card_count());
}
```

输出

```text
牌库张数: 0
手牌张数: 1
```
