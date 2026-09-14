[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **discard_top_deck_card**

# givm::player_entity::discard_top_deck_card

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr void discard_top_deck_card() const requires is_mutable;
```

丢弃牌库顶的一张牌，并移除该牌携带的状态。

## 返回值

（无）

## 注意

牌库必须非空。该操作本身不广播弃牌或状态离场事件。

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
    player.add_deck_card(definition, {});
    player.discard_top_deck_card();
    std::println("牌库剩余张数: {}", player.deck_card_count());
}
```

输出

```text
牌库剩余张数: 0
```
