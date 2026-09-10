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

## 成员函数

| | |
| --- | --- |
| [`execute`](replace_cards_both/execute.md) | 让双方分别选择开局需要替换的手牌 |

## 注意

等待输入时，栈顶可按 `top<selector, stage_t>()` 取得选择槽和需保留的尾部状态。两次提交分别指定不同玩家；选择位按该方当前有效手牌的遍历顺序。空选择表示全部保留。新牌优先避开本次换回的同名牌；这个双方开局替换过程不发出 card_drawn。

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
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    for(int submission = 0; submission < 2; ++submission)
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
