[givm](../../reference.md) / [执行](../executor.md) / **load_deck**

# givm::load_deck

定义于头文件 `<givm/executor.hpp>`

```cpp
void load_deck(table& table, const definition_library& library, const linked_deck& deck1, const linked_deck& deck2);
```

将双方已确定定义 ID 的牌组装入各自的牌库和角色区，并根据定义初始化角色状态和初始技能。返回后即可读取完整的初始角色信息，无须推进执行器。

## 参数

|  |  |
| --- | --- |
| `table` | 装载牌组的牌桌 |
| `library` | 与双方牌组定义 ID 配套的定义库 |
| `deck1` | 玩家 0 的 [`linked_deck`](../table/linked_deck.md) |
| `deck2` | 玩家 1 的 [`linked_deck`](../table/linked_deck.md) |

## 返回值

（无）

## 注意

双方的牌库和角色区必须尚未装载实体。每方卡牌按其牌组中 `cards` 的顺序装入，最后一张位于牌库顶；角色按 `characters` 的顺序装入。

角色状态采用 [`character_initial_state`](../definition/queries/character_initial_state.md) 的查询结果，初始技能通过 [`character_initial_skill`](../definition/queries/character_initial_skill.md) 从索引零开始逐项查询，首次返回无效 ID 即结束。卡牌与技能采用默认状态。

使用非限定调用 `load_deck(table, library, deck1, deck2)`。不需要装载实体的一方可传入空牌组 `{}`。

装载不使用随机源、不广播事件，也不选择出战角色或开始对局。牌桌不保存对定义库的引用，后续执行仍须传入配套定义库。

分配和查询产生的异常向外传递；异常发生前已经装载的部分不会回滚。

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
    const auto [library, id_map] = compile(sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    givm::table table{};
    givm::linked_deck deck{};
    deck.cards.push_back(id_map.get_id<givm::card_definition>("示例"));
    deck.characters.push_back(id_map.get_id<givm::character_view>("示例"));
    load_deck(table, library, deck, {});
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
