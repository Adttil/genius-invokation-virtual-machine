[givm](../../../../reference.md) / [执行](../../../executor.md) / [指令](../../instructions.md) / [replace_cards](../replace_cards.md) / **execute**

# givm::replace_cards::execute

定义于头文件 `<givm/executor/instructions/replace_cards.hpp>`

```cpp
bool execute(card_table& table, execution_context& context, random_fn& random) const;
```

[`card_table`](../../../table/card_table.md)
[`execution_context`](../../execution_context.md)
[`random_fn`](../../random_fn.md)

让指定玩家选择需要替换的手牌，将所选牌放回牌堆并抽取等量新牌。抽取时优先避开本次换回的同名牌。

由 [`executor::execute_next`](../../executor/execute_next.md) 在对局推进过程中调用。

## 参数

| | |
| --- | --- |
| `table` | 本场对局的牌桌 |
| `context` | 本次指令执行的上下文；调用须满足本指令的事件或输入约定 |
| `random` | 本次执行可使用的随机源 |

## 返回值

返回 `true` 表示可以继续自动推进，`false` 表示应停止推进，包括等待外部输入或对局结束。对局结果由 [`executor::status`](../../executor/status.md) 判断。

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
