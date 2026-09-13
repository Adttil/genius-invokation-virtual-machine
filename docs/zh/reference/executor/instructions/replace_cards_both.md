[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **replace_cards_both**

# givm::replace_cards_both

定义于头文件 `<givm/executor/instructions/replace_cards_both.hpp>`

```cpp
struct replace_cards_both;
```

双方开局换牌指令。每方提交后即可得到该方的替换结果；双方都提交后结束这项准备。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 注意

尚未接受任何一方的选择时，执行器返回 `execution_state::initial_card_selection`，通过相应的[现场视图](../execution_view/initial_card_selection.md)指定首先换牌的玩家及其选择。该方换牌完成后返回 `execution_state::card_selection`，由相应[视图](../execution_view/card_selection.md)读取剩余玩家并提交其选择。

可以任选先提交的一方；选择位按该方当前有效手牌的遍历顺序。空选择表示全部保留。新牌优先避开本次换回的同名牌；这个双方开局替换过程不发出 card_drawn。

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
        std::tuple{ givm::replace_cards_both{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    const auto a = ids.get_id<givm::card_definition>("first");
    const auto b = ids.get_id<givm::card_definition>("second");
    for(const givm::player_id player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
    {
        table[player].add_hand_card(a, {});
        table[player].insert_deck_card(0, b, {});
    }
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(table, random);
    std::bitset<givm::selection_capacity> selected{};
    selected.set(0);
    execution.view_in<givm::execution_state::initial_card_selection>().select(givm::player_id{ 1 }, selected);
    execution.run(table, random);
    const auto remaining = execution.view_in<givm::execution_state::card_selection>();
    std::println("剩余玩家为玩家 0: {}", remaining.player() == givm::player_id{ 0 });
    remaining.select(selected);
    execution.run(table, random);
    std::println("玩家 0 的手牌数量: {}", table[givm::player_id{ 0 }].hand_card_count());
    std::println("玩家 0 抽到另一种牌: {}",
        (*table[givm::player_id{ 0 }].hand_cards().begin()).definition().id().value() == b.value());
}
```

输出

```text
剩余玩家为玩家 0: true
玩家 0 的手牌数量: 1
玩家 0 抽到另一种牌: true
```
