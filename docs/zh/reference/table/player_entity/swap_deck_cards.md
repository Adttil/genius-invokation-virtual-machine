[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **swap_deck_cards**

# givm::player_entity::swap_deck_cards

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr void swap_deck_cards(size_t first, size_t second) const
    requires is_mutable;
```

交换两张牌在牌库中的位置，保留卡牌各自的身份和状态。

## 参数

|  |  |
| --- | --- |
| `first` | 第一张牌的当前牌库位置 |
| `second` | 第二张牌的当前牌库位置 |

## 返回值

（无）

## 注意

两个位置都必须小于 `deck_card_count()`。它们是顺序位置，不是卡牌 ID。

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
    player.add_deck_card(definition, {});
    player.swap_deck_cards(0, 1);
    std::println("原牌库底的牌移到顶部: {}", player.deck_cards<false>()[1].id() == first_id);
}
```

输出

```text
原牌库底的牌移到顶部: true
```
