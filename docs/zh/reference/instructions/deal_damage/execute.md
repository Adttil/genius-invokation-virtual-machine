[givm](../../../reference.md) / [指令](../../instructions.md) / [deal_damage](../deal_damage.md) / **execute**

# givm::deal_damage::execute

定义于头文件 `<givm/executor/instructions/deal_damage.hpp>`

```cpp
bool execute(card_table& table, execution_context& context, random_fn& random) const;
```

[`card_table`](../../table/card_table.md)
[`execution_context`](../../executor/execution_context.md)
[`random_fn`](../../executor/random_fn.md)

对一个角色造成伤害。它为增伤、减伤和护盾等效果提供响应时机，并在扣除生命后处理伤害所携带的元素附着与胜负判定。

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
        std::tuple{ givm::deal_damage{ .source = givm::character_id{ .player_id = givm::player_id{ 0 }, .index = 0 }, .target = { .player_id = givm::player_id{ 1 }, .index = 0 }, .value = 3, .type = givm::damage_type::physical, .flags = {} } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    const auto definition = ids.get_id<givm::character_view>("character");
    const auto attacker = table[givm::player_id{ 0 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    const auto target = table[givm::player_id{ 1 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("目标剩余生命: {}", table[target].state().health);
}
```

输出

```text
目标剩余生命: 7
```
