[givm](../../../reference.md) / [指令](../../instructions.md) / [begin_action](../begin_action.md) / **execute**

# givm::begin_action::execute

定义于头文件 `<givm/executor/instructions/begin_action.hpp>`

```cpp
bool execute(card_table& table, execution_context& context, random_fn& random) const;
```

[`card_table`](../../table/card_table.md)
[`execution_context`](../../executor/execution_context.md)
[`random_fn`](../../executor/random_fn.md)

开始本回合的行动阶段，让玩家依次选择行动，直到双方都宣布结束。

由 [`executor::execute_next`](../../executor/executor/execute_next.md) 在对局推进过程中调用。

## 参数

| | |
| --- | --- |
| `table` | 本场对局的牌桌 |
| `context` | 本指令所属的执行现场，须满足所属指令的事件或输入约定 |
| `random` | 本次执行可使用的随机源 |

## 返回值

返回 `true` 表示可以继续自动推进，`false` 表示应停止推进，包括等待外部输入或对局结束。对局结果由 [`executor::status`](../../executor/executor/status.md) 判断。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct character_source
{
    using definition_category = givm::character_view;
    struct definition_type {};
    std::string_view name() const { return "character"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::program_entry<givm::character_initialization> handle(
        const definition_type&, const givm::character_view&,
        givm::character_initialization& event, const givm::card_table&, givm::random_fn&)
    {
        event.state = { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
        return givm::program_entry<givm::character_initialization>::null();
    }
};

int main()
{
    character_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::begin_action{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    const auto definition = ids.get_id<givm::character_view>("character");
    const auto attacker = table[givm::player_id{ 0 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    const auto target = table[givm::player_id{ 1 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    table[givm::player_id{ 0 }].state().active_character = attacker;
    table[givm::player_id{ 1 }].state().active_character = target;
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    int declarations = 0;
    while(execution.status() == givm::game_result::no_result)
    {
        // 当前玩家宣布本回合结束。
        auto&& [request, preserved] = execution.stack().top<givm::action_request, givm::stage_t>();
        request = { .request_kind = givm::action_request_kind::do_action,
                    .action_kind = givm::action_kind::declare_round_end };
        ++declarations;
        while(execution.execute_next(table, random)) {}
    }
    std::println("双方结束声明次数: {}", declarations);
}
```

输出

```text
双方结束声明次数: 2
```
