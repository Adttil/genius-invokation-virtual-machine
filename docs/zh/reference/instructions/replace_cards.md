[givm](../../reference.md) / [指令](../instructions.md) / **replace_cards**

# givm::replace_cards

定义于头文件 `<givm/executor/instructions/replace_cards.hpp>`

```cpp
struct replace_cards;
```

让指定玩家选择需要替换的手牌，将所选牌放回牌堆并抽取等量新牌。抽取时优先避开本次换回的同名牌。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，可用于不限定事件的操作 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../table/player_id.md) | 替换手牌的玩家 |

## 成员函数

| | |
| --- | --- |
| [`execute`](replace_cards/execute.md) | 让指定玩家选择需要替换的手牌，将所选牌放回牌堆并抽取等量新牌 |

## 注意

等待输入时，栈顶可按 `top<selector, stage_t>()` 取得选择槽和需保留的尾部状态。保持 `selector.player` 为指定玩家，在 `selected` 中标记要换回的手牌；位序按当前有效手牌的遍历顺序。空选择表示保留全部手牌。替换完成后，逐张发出 [`card_drawn`](../events/card_drawn.md)。

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
        std::tuple{ givm::replace_cards{ .player = givm::player_id{ 0 } } },
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
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    for(int submission = 0; submission < 1; ++submission)
    {
        auto&& [input, preserved] = execution.stack().top<givm::selector, givm::stage_t>();
        input.selected.set(0);
        while(execution.execute_next(table, random)) {}
    }
    std::println("玩家 0 的手牌数量: {}", table[givm::player_id{ 0 }].hand_card_count());
    std::println("玩家 0 抽到另一种牌: {}",
        (*table[givm::player_id{ 0 }].hand_cards().begin()).definition().id().value() == b.value());
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
| [`card_drawn`](../events/card_drawn.md) | 抽到一张牌 |
