[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **draw_cards**

# givm::draw_cards

定义于头文件 `<givm/definition.hpp>`

```cpp
struct draw_cards;
```

抽牌命令，指定玩家从牌堆顶抽取一定数量的牌。它可用于开局发牌、回合结束抽牌或卡牌效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `count` | `std::uint32_t` | 尝试抽取的牌数 |
| `player` | [`relative_player`](relative_player.md) | 相对于当前行动玩家的抽牌方，初始为 current |

## 注意

牌堆耗尽后停止抽取。达到手牌上限后，仍继续从牌堆移走本次应抽的牌，但这些牌不进入手牌。先完成本次所有抽牌，再逐张发出 [`card_drawn`](../events/card_drawn.md)，只通知实际进入手牌的牌。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;
    struct definition_type {};
    std::string_view source_name;
    std::string_view name() const { return source_name; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    card_source first{ "first" };
    card_source second{ "second" };
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(first, second);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::draw_cards{ .count = 2 } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    auto player = table[givm::player_id{ 0 }];
    const auto card = ids.get_id<givm::card_definition>("first");
    load_deck(table, library, givm::linked_deck{ .cards = { card, card } }, {});
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    std::println("手牌数量: {}", player.hand_card_count());
    std::println("牌堆数量: {}", player.deck_card_count());
}
```

输出

```text
手牌数量: 2
牌堆数量: 0
```

## 参阅

| | |
| --- | --- |
| [`card_drawn`](../events/card_drawn.md) | 一张牌抽取完成后的通知 |
