[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **clean_up**

# givm::player_entity::clean_up

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr void clean_up() const requires is_mutable;
```

清理该玩家已经移除的卡牌、角色、支援、召唤物和出战状态。

## 返回值

（无）

## 注意

该玩家的实体 ID、访问对象和范围可能失效；清理后须重新取得。此函数不执行牌桌层面的卡牌状态清理，需要完整清理时使用 [`card_table::clean_up`](../card_table/clean_up.md)。

## 示例

```cpp
#include <print>
#include <ranges>
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
    const auto card = player.add_hand_card(definition, {});
    card.erase();
    player.clean_up();
    std::println("清理后全部手牌位置: {}", std::ranges::distance(player.hand_cards<false>()));
}
```

输出

```text
清理后全部手牌位置: 0
```
