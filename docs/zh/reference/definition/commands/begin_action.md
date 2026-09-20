[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **begin_action**

# givm::begin_action

定义于头文件 `<givm/definition.hpp>`

```cpp
struct begin_action;
```

行动阶段的处理命令，涵盖玩家选择行动至双方宣布结束的过程。

## 注意

先发出 [`action_phase_started`](../events/action_phase_started.md)，每次选择行动前发出 [`before_action`](../events/before_action.md)。支持使用技能、打出手牌、主动切换出战角色和宣布结束；当前行动方必须已有出战角色。双方均宣布结束后，本命令才结束行动阶段。

等待选择行动时，执行器返回 `execution_state::action_selection`，通过相应的[现场视图](../../executor/execution_view/action_selection.md)预览费用、检查或选择行动。出牌选择当前行动玩家的有效手牌，并提供支付骰子及至多两个目标；目标参数默认为空 span，超过两个的元素忽略。目标和用牌条件由牌定义决定。主动切换选择当前行动玩家存活、非出战的角色。调用方保证支付满足费用、骰子持有数量及出战角色充能；宣布结束无需支付骰子。

调用方必须通过 `use_skill`、`play_card`、`switch_active_character` 或 `declare_round_end` 提供本次行动输入后，才能再次调用 `step`。费用预览、支付检查与目标检查不提供行动输入；等待玩家决定期间由上层保留当前现场。

技能、出牌与切换分别使用从零开始的候选索引。技能通过 [`skill_count`](../../executor/execution_view/action_selection/skill_count.md) 查询数量、[`skill_id`](../../executor/execution_view/action_selection/skill_id.md) 查询对应技能 ID。通过 [`card_count`](../../executor/execution_view/action_selection/card_count.md) 和 [`switch_target_count`](../../executor/execution_view/action_selection/switch_target_count.md) 查询数量，通过 [`card_id`](../../executor/execution_view/action_selection/card_id.md) 和 [`switch_target`](../../executor/execution_view/action_selection/switch_target.md) 查询对应实体 ID；技能和牌的效果目标仍使用 ID。

技能候选仅包含出战角色中支持 [`skill_effect`](../events/skill_effect.md) 的有效技能。通过 [`calculate_skill_cost`](../../executor/execution_view/action_selection/calculate_skill_cost.md) 查询费用，按需独立进行 [`skill_payment_validate`](../../executor/execution_view/action_selection/skill_payment_validate.md) 和 [`skill_targets_validate`](../../executor/execution_view/action_selection/skill_targets_validate.md)，再通过 [`use_skill`](../../executor/execution_view/action_selection/use_skill.md) 提交技能、骰子及目标。支付后广播 [`skill_will_be_used`](../events/skill_will_be_used.md)，未取消时执行技能自身效果，之后均广播 [`skill_used`](../events/skill_used.md)；取消效果不撤销本次使用或支付。行动速度采用生效前响应的最终结果。

通过 [`calculate_card_cost`](../../executor/execution_view/action_selection/calculate_card_cost.md) 同步计算出牌费用，通过 [`card_payment_validate`](../../executor/execution_view/action_selection/card_payment_validate.md) 与 [`card_targets_validate`](../../executor/execution_view/action_selection/card_targets_validate.md) 分别检查支付及用牌条件。目标检查按 span 中的目标数量分步进行，允许检查空选择，告知当前选择是否有效、能否完成或继续；检查第二目标时可假设第一目标合法。两项检查相互独立，由调用方按需使用。目标检查通过 [`card_target_validation`](../queries/card_target_validation.md) 返回结果，不接收随机源。费用响应不得使用随机数，调用随机函数属于未定义行为。

[`play_card`](../../executor/execution_view/action_selection/play_card.md) 可采用已完整计算的费用，也可同步计算报价后选择出牌；不会自动检查支付或目标。下一次推进先让牌离手，再执行已确认的费用效果、扣除骰子与充能，再依次处理骰子移除和充能变化通知，随后广播 [`card_will_be_played`](../events/card_will_be_played.md)。未被反制时执行本牌的 [`card_effect`](../events/card_effect.md)，之后均广播 [`card_played`](../events/card_played.md)。反制只取消原效果，不退还费用或撤销离手。最后按报价确定的行动速度保留或交接行动权。

通过 [`calculate_switch_cost`](../../executor/execution_view/action_selection/calculate_switch_cost.md) 可以同步预览切换至指定角色的费用，无需推进执行器或传入随机源。费用响应不得使用随机数，调用随机函数属于未定义行为；目标为只读。完整报价后可调用 [`switch_payment_validate`](../../executor/execution_view/action_selection/switch_payment_validate.md)，依次检查骰子是否匹配费用、持有数量是否足够、非零充能费用的类型是否匹配及出战角色充能是否足够。

通过 [`switch_active_character`](../../executor/execution_view/action_selection/switch_active_character.md) 选择切换时，可采用已经计算的费用，也可传入定义库和牌桌，在本次调用中同步计算报价后提交。两种重载均由下一次推进执行已确认的费用效果、支付及切换，不自动检查支付是否合法。采用已计算费用时，由调用方保证该角色已经完整报价。同一行动窗口内每个候选只允许计算一次报价，可重复读取结果；带定义库与牌桌的提交重载仅用于尚未报价的候选，库不检查此约定。

在 [`compile_mode::observed`](../../executor/compile_mode.md) 模式下推进主动切人时，在写入新出战角色之前返回 `execution_state::active_character_changed`。相应[视图](../../executor/execution_view/active_character_changed.md)给出目标，牌桌仍可读取原出战角色；随后推进先完成设置，再处理变更响应。到达此现场前，已确认的费用响应、骰子与充能支付及相应变化响应均已完成。

以 [`compile_mode::observed`](../../executor/compile_mode.md) 编译时，每次新的行动机会先返回 `execution_state::action_started`，随后才处理 `before_action`。快速行动不结束当前机会；战斗行动结束后，即使另一方已经宣布结束、仍由当前玩家行动，也会报告新的行动机会。宣布结束时先返回 `execution_state::round_end_declared`，此时牌桌上的 `active_player` 仍是宣布结束的一方，随后推进才处理其结束响应。

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

    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
    }
};

struct card_source
{
    using definition_category = givm::card_definition;
    struct definition_type {};
    std::string_view name() const { return "card"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    character_source source{};
    card_source card{};
    givm::definition_source_library sources{};
    sources.add(source);
    sources.add(card);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::set_active_character{ .target = givm::character_id{ givm::player_id{ 0 }, 0 } }, givm::set_active_character{ .target = givm::character_id{ givm::player_id{ 1 }, 0 } }, givm::draw_cards{ .count = 1 }, givm::begin_action{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    const auto definition = ids.get_id<givm::character_view>("character");
    const auto card_definition = ids.get_id<givm::card_definition>("card");
    load_deck(table, library,
        givm::linked_deck{
            .cards = { card_definition }, .characters = { definition, definition }
        },
        givm::linked_deck{ .characters = { definition } });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    auto state = execution.step(library, table, random);
    if(state == givm::execution_state::action_selection)
    {
        const auto action = execution.view_in<givm::execution_state::action_selection>();
        // 显示候选只需索引，不必先计算费用。
        std::println("第一个切换候选是后备角色: {}",
            action.switch_target(0) == givm::character_id{ givm::player_id{ 0 }, 1 });
        std::println("第一个出牌候选采用已加载的定义: {}",
            table[action.card_id(0)].definition_id() == card_definition);
    }
    int declarations = 0;
    while(state == givm::execution_state::action_selection)
    {
        // 当前玩家宣布本回合结束。
        execution.view_in<givm::execution_state::action_selection>().declare_round_end();
        ++declarations;
        state = execution.step(library, table, random);
    }
    std::println("双方结束声明次数: {}", declarations);
}
```

输出

```text
第一个切换候选是后备角色: true
第一个出牌候选采用已加载的定义: true
双方结束声明次数: 2
```

## 参阅

| | |
| --- | --- |
| [`action_phase_started`](../events/action_phase_started.md) | 本回合行动阶段开始的通知 |
| [`before_action`](../events/before_action.md) | 当前行动玩家选择行动前的事件 |
| [`cost_of_switch`](../events/cost_of_switch.md) | 主动切换出战角色的费用计算事件 |
| [`round_end_declared`](../events/round_end_declared.md) | 玩家宣布本回合结束的通知 |
