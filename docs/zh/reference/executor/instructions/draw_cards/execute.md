[givm](../../../../reference.md) / [执行](../../../executor.md) / [指令](../../instructions.md) / [draw_cards](../draw_cards.md) / **execute**

# givm::draw_cards::execute

定义于头文件 `<givm/executor/instructions/draw_cards.hpp>`

```cpp
bool execute(card_table& table, execution_context& context, random_fn& random) const;
```

[`card_table`](../../../table/card_table.md)
[`execution_context`](../../execution_context.md)
[`random_fn`](../../random_fn.md)

让一名玩家从牌堆顶抽取指定数量的牌。它可用于开局发牌、回合结束抽牌或卡牌效果。

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
