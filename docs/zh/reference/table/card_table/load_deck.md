[givm](../../../reference.md) / [牌桌](../../table.md) / [card_table](../card_table.md) / **load_deck**

# givm::card_table::load_deck

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr void load_deck(player_id player, const linked_deck& deck);
```

将已确定定义 ID 的牌组装入玩家的牌库和角色区。

## 参数

|  |  |
| --- | --- |
| `player` | 装载牌组的玩家 |
| `deck` | 保存卡牌与角色定义 ID 的 [`linked_deck`](../linked_deck.md) |

## 返回值

（无）

## 注意

该玩家的牌库和角色区必须尚未装载实体。卡牌按 `deck.cards` 的顺序装入，最后一张位于牌库顶；角色按 `deck.characters` 的顺序装入。该操作不执行角色初始化或开始对局，也不查询定义库。后续执行时须显式传入与这些定义 ID 配套的定义库。

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
    const example_source<givm::character_view> character_source{};
    sources.add(card_source, character_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    givm::linked_deck deck{};
    deck.cards.push_back(id_map.get_id<givm::card_definition>("示例"));
    deck.characters.push_back(id_map.get_id<givm::character_view>("示例"));
    table.load_deck(givm::player_id{ 0 }, deck);
    const auto player = table[givm::player_id{ 0 }];
    std::println("牌库张数: {}", player.deck_card_count());
    std::println("角色数量: {}", std::ranges::distance(player.characters()));
}
```

输出

```text
牌库张数: 1
角色数量: 1
```
