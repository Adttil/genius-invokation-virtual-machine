[givm](../../../../reference.md) / [执行](../../../executor.md) / [指令](../../instructions.md) / [absorb_damage_by_count](../absorb_damage_by_count.md) / **execute**

# givm::absorb_damage_by_count::execute

定义于头文件 `<givm/executor/instructions/absorb_damage_by_count.hpp>`

```cpp
bool execute(card_table& table, execution_context& context, random_fn&) const;
```

[`card_table`](../../../table/card_table.md)
[`execution_context`](../../execution_context.md)
[`random_fn`](../../random_fn.md)

用当前响应实体的次数或层数抵挡伤害。它适用于护盾等需要随着吸收伤害而消耗计数的效果。

由 [`executor::execute_next`](../../executor/execute_next.md) 在对局推进过程中调用。

## 参数

| | |
| --- | --- |
| `table` | 本场对局的牌桌 |
| `context` | 本次指令执行的上下文；调用须满足本指令的事件或输入约定 |

## 返回值

返回 `true` 表示可以继续自动推进，`false` 表示应停止推进，包括等待外部输入或对局结束。对局结果由 [`executor::status`](../../executor/status.md) 判断。

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

struct shield_source
{
    using definition_category = givm::combat_status_view;
    struct definition_type { givm::program_entry<givm::damage_effect> response; };
    std::string_view name() const { return "shield"; }
    definition_type compile(givm::definition_compile_context& context) const
    {
        return { context.add_program<givm::damage_effect>(
            std::tuple{ givm::absorb_damage_by_count{ .maximum_count = 2 } }) };
    }
    static givm::program_entry<givm::damage_effect> handle(
        const definition_type& data, const givm::combat_status_view& self,
        givm::damage_effect& event, const givm::card_table&, givm::random_fn&)
    {
        if(event.target.player_id == self.player().id()
            && !event.flags.contains(givm::damage_flag_bits::ignore_shield)
            && event.value != 0 && self.state().count != 0)
        {
            return data.response;
        }
        return givm::program_entry<givm::damage_effect>::null();
    }
};

int main()
{
    character_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    shield_source shield_definition{};
    sources.add(shield_definition);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::deal_damage{ .source = givm::character_id{ .player_id = givm::player_id{ 0 }, .index = 0 }, .target = { .player_id = givm::player_id{ 1 }, .index = 0 }, .value = 3, .type = givm::damage_type::physical, .flags = {} } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    const auto definition = ids.get_id<givm::character_view>("character");
    const auto attacker = table[givm::player_id{ 0 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    const auto target = table[givm::player_id{ 1 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    const auto shield = table[givm::player_id{ 1 }].add(
        ids.get_id<givm::combat_status_view>("shield"), { .count = 3 });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("角色剩余生命: {}", table[target].state().health);
    std::println("护盾剩余计数: {}", shield.state().count);
}
```

输出

```text
角色剩余生命: 9
护盾剩余计数: 1
```
