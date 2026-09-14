[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **replace_cards**

# givm::replace_cards

定义于头文件 `<givm/executor/instructions/replace_cards.hpp>`

```cpp
struct replace_cards;
```

单方换牌指令，包括选择待换手牌、放回所选牌及抽取等量新牌。抽取时优先避开本次换回的同名牌。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 替换手牌的玩家 |

## 注意

等待输入时，执行器返回 `execution_state::card_selection`，通过相应的[现场视图](../execution_view/card_selection.md)提交选择。选择位按当前有效手牌的遍历顺序，空选择表示保留全部手牌。替换完成后，逐张发出 [`card_drawn`](../events/card_drawn.md)。

## 示例

```cpp
#include <bitset>
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
        std::tuple{ givm::draw_cards{ .count = 1 }, givm::draw_cards{ .count = 1, .player = givm::relative_player::other }, givm::replace_cards{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{};
    const auto a = ids.get_id<givm::card_definition>("first");
    const auto b = ids.get_id<givm::card_definition>("second");
    for(const givm::player_id player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
    {
        table.load_deck(player, givm::linked_deck{ .cards = { b, a } });
    }
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(library, table, random);
    std::bitset<givm::selection_capacity> selected{};
    selected.set(0);
    execution.view_in<givm::execution_state::card_selection>().select(selected);
    execution.run(library, table, random);
    std::println("玩家 0 的手牌数量: {}", table[givm::player_id{ 0 }].hand_card_count());
    std::println("玩家 0 抽到另一种牌: {}",
        (*table[givm::player_id{ 0 }].hand_cards().begin()).definition_id().value() == b.value());
}
```

输出

```text
玩家 0 的手牌数量: 1
玩家 0 抽到另一种牌: true
```

## 参阅

| | |
| --- | --- |
| [`card_drawn`](../events/card_drawn.md) | 一张牌抽取完成后的通知 |
