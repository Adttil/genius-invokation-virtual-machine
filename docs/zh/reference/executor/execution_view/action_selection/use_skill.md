[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **use_skill**

# givm::execution_view<execution_state::action_selection>::use_skill

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class TRandom>
execution_state use_skill(
    const definition_library& library, table& card_table, TRandom& random_source,
    std::size_t skill_index, const dice_counts& paid_dice, std::span<const skill_target_id> targets = {}
) const;
```

对具体的使用技能选择报价，并立即采用结果推进行动。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 可调用并返回 `std::uint32_t` 的随机源类型。 |

## 参数

| | |
| --- | --- |
| `library` | 与当前现场及牌桌配套的定义库。 |
| `card_table` | 当前行动发生的牌桌。 |
| `random_source` | 后续执行使用的随机源；报价不消耗随机数。 |
| `skill_index` | 从零开始的当前行动候选索引。 |
| `paid_dice` | 原选支付骰子的颜色及数量。 |
| `targets` | 完整的目标选择；省略表示两个空槽。 |

## 返回值

推进后到达的执行现场。

## 异常

Debug 检查现场、候选或报价标识的有效性，并以结构化异常报告误用。费用响应抛出的异常直接传递；失败报价不能采用或在当前窗口重算。Release 不执行这些输入校验。

## 注意

相当于先调用 `calculate_skill_cost`，再使用返回标识调用 `use_skill_with_cached_cost`。只用于尚未报价的操作与目标组合。需要预览费用、检查支付或修正输入时，先显式报价并保存标识。

如果直接接口在完成报价后抛出 Debug 支付异常，它不会返回报价标识，也不回滚报价；调用方不能在本窗口重算这个组合。

目标最多两个，超过两项的内容忽略。``null` 空类别` 表示空槽，遇到首个空槽即结束目标序列；报价无需额外保存目标数量。完整目标必须合法，分步检查仍由相应的 `*_targets_validate` 提供。

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
    static constexpr auto category = givm::definition_category::skill;
    std::string_view name() const { return "example_skill"; }
    int compile(givm::definition_compile_context&) const { return 0; }

    static givm::action_cost_requirement query(const int&, const givm::skill_initial_cost&)
    {
        return { .speed = givm::action_speed::combat, .energy = 1 };
    }

    static givm::normal_effect handle(
        const int&, givm::this_skill_use&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
    {
        return {};
    }
};

struct character_source
{
    static constexpr auto category = givm::definition_category::character;
    using definition_type = givm::optional_definition_id<givm::definition_category::skill>;

    std::string_view name() const { return "example_character"; }
    auto skill_dependencies() const { return std::array{ "example_skill" }; }
    definition_type compile(givm::definition_compile_context& context) const
    {
        return context.resolve_id<givm::definition_category::skill>("example_skill");
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
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(skill)) return 1;
    if(not sources.add(character)) return 1;
    auto library_result = compile(sources, basics,
        std::tuple{
            givm::select_active_character_both{},
            givm::begin_action{}, givm::settle{}, givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);

    givm::table table{};
    const auto character_definition = ids.get_id<givm::definition_category::character>("example_character");
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
    const auto user = action.skill_id(0).character_id();
    const auto quote_1 = action.calculate_skill_cost(library, table, 0);
    std::println("支付合法: {}",
        action.skill_payment_validate(table, quote_1, {}) == givm::skill_payment_validation::valid);
    std::println("无需目标: {}",
        action.skill_targets_validate(library, table, 0) == givm::target_validation::valid_complete);
    state = action.use_skill_with_cached_cost(library, table, random, quote_1, {});
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
