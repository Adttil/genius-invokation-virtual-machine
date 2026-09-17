[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **use_skill**

# givm::execution_view<execution_state::action_selection>::use_skill

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void use_skill(
    std::size_t skill_index, const dice_counts& paid_dice, std::span<const skill_target_id> targets = {}
) const noexcept;

void use_skill(
    const definition_library& library, const table& card_table,
    std::size_t skill_index, const dice_counts& paid_dice, std::span<const skill_target_id> targets = {}
) const;
```

选择出战角色要使用的技能、技能目标与支付骰子。下一次推进才开始结算这次技能行动。

## 参数

| | |
| --- | --- |
| `skill_index` | 从零开始的技能候选索引，须小于 [`skill_count()`](skill_count.md)。 |
| `paid_dice` | 本次支付的骰子，须满足采用的费用及持有数量。 |
| `targets` | 按选择顺序提供的目标 ID，默认空 span 表示不选目标。只采用前两个元素，多余元素忽略；缺少的位置补为 `std::monostate`。 |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |

## 返回值

（无）

## 异常

带定义库与牌桌的重载会传递报价响应的异常。报价失败时不提交本次选择；该技能须重新完整报价成功后才能检查支付或采用。

## 注意

不带定义库与牌桌的重载采用已完整计算的费用。另一重载同步重新报价后填写选择；两者都不自动检查支付、目标或其他使用条件。调用方可以独立使用 [`skill_payment_validate`](skill_payment_validate.md) 与分步的 [`skill_targets_validate`](skill_targets_validate.md)，并负责保证输入合法、当前选择允许完成。

充能按费用要求自动从出战角色扣除，不需要另行选择支付量。目标由技能定义解释，未使用的位置忽略；无需目标时可直接调用 `use_skill(skill_index, paid_dice)`。本操作复制采用的目标 ID，调用完成后无需保留传入的目标范围。

本操作不推进执行器，也不修改牌桌。下一次 [`step`](../../executor/step.md) 先执行确认的费用效果，再扣除骰子与充能，依次处理骰子移除和充能变化通知，然后广播 [`skill_will_be_used`](../../../definition/events/skill_will_be_used.md)。若效果未被取消，则执行该技能的 [`skill_effect`](../../../definition/events/skill_effect.md)；之后均广播 [`skill_used`](../../../definition/events/skill_used.md)。取消效果不退还支付，也不撤销本次技能使用。

本次行动采用 `skill_will_be_used` 响应完成后的行动速度：快速行动保留行动权，战斗行动按行动阶段规则交接。整个流程由 [`begin_action`](../../../definition/commands/begin_action.md) 处理。

## 示例

```cpp
#include <array>
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct skill_source
{
    using definition_category = givm::skill_view;
    std::string_view name() const { return "example_skill"; }
    int compile(givm::definition_compile_context&) const { return 0; }

    static givm::action_cost_requirement query(const int&, const givm::skill_initial_cost&)
    {
        return { .energy = 1, .speed = givm::action_speed::combat };
    }

    static givm::handler_program_entry_t<givm::skill_effect> handle(
        const int&, const givm::skill_view&, givm::skill_effect&, const givm::table&, givm::random_fn&
    )
    {
        return givm::handler_program_entry_t<givm::skill_effect>::null();
    }
};

struct character_source
{
    using definition_category = givm::character_view;
    using definition_type = givm::definition_id<givm::skill_view>;

    std::string_view name() const { return "example_character"; }
    auto skill_dependencies() const { return std::array{ "example_skill" }; }
    definition_type compile(givm::definition_compile_context& context) const
    {
        return context.resolve_id<givm::skill_view>("example_skill");
    }

    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .max_energy = 2, .health = 10, .energy = 2 };
    }

    static definition_type query(const definition_type& skill, const givm::character_initial_skill& parameters)
    {
        return parameters.skill_index == 0 ? skill : definition_type{};
    }
};

int main()
{
    skill_source skill{};
    character_source character{};
    givm::definition_source_library sources{};
    sources.add(skill);
    sources.add(character);
    const auto [library, ids] = compile(sources,
        std::tuple{
            givm::initialize_characters{ .player = givm::player_id{ 0 } },
            givm::initialize_characters{ .player = givm::player_id{ 1 } },
            givm::set_active_character{ .target = { givm::player_id{ 0 }, 0 } },
            givm::set_active_character{ .target = { givm::player_id{ 1 }, 0 } },
            givm::begin_action{}, givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);

    givm::table table{};
    const auto character_definition = ids.get_id<givm::character_view>("example_character");
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .characters = { character_definition } });
    table.load_deck(givm::player_id{ 1 }, givm::linked_deck{ .characters = { character_definition } });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    auto state = execution.step(library, table, random);
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    std::println("技能候选数量: {}", action.skill_count());
    const auto user = action.skill_id(0).character_id;
    action.calculate_skill_cost(library, table, 0);
    std::println("支付合法: {}",
        action.skill_payment_validate(table, 0, {}) == givm::skill_payment_validation::valid);
    std::println("无需目标: {}",
        action.skill_targets_validate(library, table, 0) == givm::target_validation::valid_complete);
    action.use_skill(0, {});
    state = execution.step(library, table, random);
    std::println("使用后的充能: {}", table[user].state().energy);
    while(state == givm::execution_state::action_selection)
    {
        execution.view_in<givm::execution_state::action_selection>().declare_round_end();
        state = execution.step(library, table, random);
    }
}
```

输出

```text
技能候选数量: 1
支付合法: true
无需目标: true
使用后的充能: 1
```
