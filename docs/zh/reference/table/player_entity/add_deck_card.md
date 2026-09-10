[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **add_deck_card**

# givm::player_entity::add_deck_card

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr deck_card_entity<TStorage> add_deck_card(
    definition_id<card_definition> definition_id, const card_state& state
) const requires is_mutable;
```

在该玩家牌库顶加入一张新卡牌。

## 参数

|  |  |
| --- | --- |
| [`definition_id`](../../definition/definition_id.md) | 配套定义库中的卡牌定义 ID |
| `state` | 新卡牌的初始状态 |

## 返回值

新加入牌库的 [`deck_card_entity`](../deck_card_entity.md)。

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
    const auto card = player.add_deck_card(definition, {});
    std::println("牌库张数: {}", player.deck_card_count());
    std::println("新卡牌有效: {}", card.is_valid());
}
```

输出

```text
牌库张数: 1
新卡牌有效: true
```
