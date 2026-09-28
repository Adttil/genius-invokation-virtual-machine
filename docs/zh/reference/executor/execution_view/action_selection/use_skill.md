[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **use_skill**

# givm::execution_view<execution_state::action_selection>::use_skill

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
execution_state use_skill(
    const definition_library& library, table& card_table, TRandom& random,
    std::size_t skill_index, const dice_counts& paid_dice, std::span<const skill_target_id> targets = {}
) const;

template<class TRandom>
execution_state use_skill_with_cached_cost(
    const definition_library& library, table& card_table, TRandom& random,
    std::size_t skill_index, const dice_counts& paid_dice, std::span<const skill_target_id> targets = {}
) const;
```

选择出战角色要使用的技能、技能目标与支付骰子。提交后立即推进，返回下一处暂停现场。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 非 `const`、非 `volatile` 的可调用对象类型，其无参数调用结果可隐式转换为 `std::uint32_t`。 |

## 参数

| | |
| --- | --- |
| `skill_index` | 从零开始的技能候选索引，须小于 [`skill_count()`](skill_count.md)。 |
| `paid_dice` | 本次支付的骰子，须满足采用的费用及持有数量。 |
| `targets` | 按选择顺序提供的目标 ID，默认空 span 表示不选目标。只采用前两个元素，多余元素忽略；缺少的位置补为 `std::monostate`。 |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `random` | 本次推进使用的随机源，以左值传入。 |

## 返回值

推进后到达的 [`execution_state`](../../execution_state.md)，可能是下一处输入、观察或终局现场。

## 异常

未定义 `NDEBUG` 时，现场已失效或种类错误会抛出 [`execution_view_error`](../../execution_view_error.md)；输入合法性结果失败时抛出 [`view_input_error`](../../view_input_error.md)；实体 ID 越界或已移除的诊断沿用 [`command_input_error`](../../command_input_error.md)。这些检查均在填写选择和开始推进之前。进入执行后，定义源、随机源及命令检查产生的异常向外传播，执行不提供回滚。

计算费用时的响应异常也向外传播。报价失败后，该候选在本现场不能重新报价或使用缓存。

## 注意

若即时报价已经成功，而后续 Debug 输入检查失败，报价仍然保留；修正输入后应调用 `use_skill_with_cached_cost`，不能重新报价。

出战角色受控时不能使用技能；可通过 [`is_controlled`](is_controlled.md) 独立查询。Debug 提交会检查控制状态、支付与目标，Release 由调用方保证这些条件成立。

同一行动窗口内，每个候选只允许计算一次费用。`use_skill` 同步计算报价后提交，只用于尚未报价的候选；`use_skill_with_cached_cost` 使用已经完整计算的报价与对应支付效果，不重新计算。Debug 检查报价状态，Release 由调用方保证。

两种提交方式均在 Debug 下检查支付、目标及使用条件；目标按选择前缀依次验证，最终选择须允许完成。Release 不执行这些检查。调用方可以独立使用 [`skill_payment_validate`](skill_payment_validate.md) 与分步的 [`skill_targets_validate`](skill_targets_validate.md)，并负责保证输入合法、当前选择允许完成。

充能按费用要求自动从出战角色扣除，不需要另行选择支付量。目标由技能定义解释，未使用的位置忽略；无需目标时可直接调用 `use_skill(library, card_table, random, skill_index, paid_dice)`。本操作复制采用的目标 ID，调用完成后无需保留传入的目标范围。

本操作提交并推进，先执行确认的费用效果，再扣除骰子与充能，依次处理骰子移除和充能变化通知，然后广播 [`skill_will_be_used`](../../../definition/events/skill_will_be_used.md)。若效果未被取消，则执行该技能的 [`skill_effect`](../../../definition/events/skill_effect.md)；之后均广播 [`skill_used`](../../../definition/events/skill_used.md)。取消效果不退还支付，也不撤销本次技能使用。

技能使用不会自动增加充能。需要获得充能的技能应在自身效果程序中显式安排 [`modify_energy`](../../../definition/commands/modify_energy.md)，因此获得充能的时机由技能效果决定。

本次行动采用 `skill_will_be_used` 响应完成后的行动速度：快速行动保留行动权，战斗行动按行动阶段规则交接。整个流程由 [`begin_action`](../../../definition/commands/begin_action.md) 处理。

## 示例

```cpp
#include <utility>
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
        return { .speed = givm::action_speed::combat, .energy = 1 };
    }

    static givm::program_entry handle(
        const int&, const givm::skill_view&, givm::skill_effect&, givm::handle_context& context)
    {
        return {};
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
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(skill)) return 1;
    if(not sources.add(character)) return 1;
    auto library_result = compile(sources, basics,
        std::tuple{
            givm::select_active_character_both{},
            givm::begin_action{}, givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);

    givm::table table{};
    const auto character_definition = ids.get_id<givm::character_view>("example_character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { character_definition } },
        givm::linked_deck{ .characters = { character_definition } });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 0 }, 0 });
    auto state = execution.view_in<givm::execution_state::remaining_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 1 }, 0 });
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    std::println("技能候选数量: {}", action.skill_count());
    const auto user = action.skill_id(0).character_id;
    action.calculate_skill_cost(library, table, 0);
    std::println("支付合法: {}",
        action.skill_payment_validate(table, 0, {}) == givm::skill_payment_validation::valid);
    std::println("无需目标: {}",
        action.skill_targets_validate(library, table, 0) == givm::target_validation::valid_complete);
    state = action.use_skill_with_cached_cost(library, table, random, 0, {});
    std::println("使用后的充能: {}", table[user].state().energy);
    while(state == givm::execution_state::action_selection)
    {
        state = execution.view_in<givm::execution_state::action_selection>().declare_round_end(library, table, random);
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
