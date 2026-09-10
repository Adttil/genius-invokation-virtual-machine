[givm](../../reference.md) / [指令](../instructions.md) / **draw_cards**

# givm::draw_cards

定义于头文件 `<givm/executor/instructions/draw_cards.hpp>`

```cpp
struct draw_cards;
```

让一名玩家从牌堆顶抽取指定数量的牌。它可用于开局发牌、回合结束抽牌或卡牌效果。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，可用于不限定事件的操作 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `count` | `std::uint32_t` | 尝试抽取的牌数 |
| `player` | [`relative_player`](relative_player.md) | 相对于当前行动玩家的抽牌方，初始为 current |

## 成员函数

| | |
| --- | --- |
| [`execute`](draw_cards/execute.md) | 让一名玩家从牌堆顶抽取指定数量的牌 |

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
    givm::definition_source_library sources{};
    sources.add(first, second);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::draw_cards{ .count = 2 } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    auto player = table[givm::player_id{ 0 }];
    const auto card = ids.get_id<givm::card_definition>("first");
    player.insert_deck_card(0, card, {});
    player.insert_deck_card(1, card, {});
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
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
| [`card_drawn`](../events/card_drawn.md) | 抽到一张牌 |
