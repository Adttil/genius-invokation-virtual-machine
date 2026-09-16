[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **begin_action**

# givm::begin_action

定义于头文件 `<givm/definition.hpp>`

```cpp
struct begin_action;
```

行动阶段的处理命令，涵盖玩家选择行动至双方宣布结束的过程。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 注意

先发出 [`action_phase_started`](../events/action_phase_started.md)，每次选择行动前发出 [`before_action`](../events/before_action.md)。支持打出手牌、主动切换出战角色和宣布结束；当前行动方必须已有出战角色。双方均宣布结束后，本命令才结束行动阶段。

等待选择行动时，执行器返回 `execution_state::action_selection`，通过相应的[现场视图](../../executor/execution_view/action_selection.md)预览费用、检查或选择行动。出牌使用当前行动玩家的有效手牌 ID、固定两个目标位置与支付骰子；目标和用牌条件由牌定义决定，未使用的目标位置忽略。主动切换使用角色 ID 指定当前行动玩家存活、非出战的角色。调用方保证支付满足费用及持有数量；宣布结束无需支付骰子。

调用方必须通过 `play_card`、`switch_active_character` 或 `declare_round_end` 提供本次行动输入后，才能再次调用 `step`。费用预览、支付检查与目标检查不提供行动输入；等待玩家决定期间由上层保留当前现场。

通过 [`calculate_card_cost`](../../executor/execution_view/action_selection/calculate_card_cost.md) 同步计算出牌费用，通过 [`check_card_payment`](../../executor/execution_view/action_selection/check_card_payment.md) 与 [`check_card_targets`](../../executor/execution_view/action_selection/check_card_targets.md) 分别检查支付及用牌条件；两项检查相互独立，由调用方按需使用。费用初始化、费用响应与目标检查不得使用随机数，调用随机函数属于未定义行为。

[`play_card`](../../executor/execution_view/action_selection/play_card.md) 可采用已完整计算的费用，也可同步重新报价后选择出牌；不会自动检查支付或目标。下一次推进先让牌离手，再执行已确认的费用效果、扣骰与骰子变化响应，随后广播 [`card_will_be_played`](../events/card_will_be_played.md)。未被反制时执行本牌的 [`card_effect`](../events/card_effect.md)，之后均广播 [`card_played`](../events/card_played.md)。反制只取消原效果，不退还费用或撤销离手。最后按报价确定的行动速度保留或交接行动权。

通过 [`calculate_switch_cost`](../../executor/execution_view/action_selection/calculate_switch_cost.md) 可以同步预览切换至指定角色的费用，无需推进执行器或传入随机源。费用响应不得使用随机数，调用随机函数属于未定义行为；目标为只读。完整报价后可调用 [`check_switch_payment`](../../executor/execution_view/action_selection/check_switch_payment.md)，检查所选骰子是否匹配费用且持有数量足够。

通过 [`switch_active_character`](../../executor/execution_view/action_selection/switch_active_character.md) 选择切换时，可采用已经计算的费用，也可传入定义库和牌桌，在本次调用中同步重新报价后提交。两种重载均由下一次推进执行已确认的费用效果、支付及切换，不自动检查支付是否合法。采用已计算费用时，由调用方保证该角色已经完整报价。

在 [`compile_mode::observed`](../../executor/compile_mode.md) 模式下推进主动切人时，在写入新出战角色之前返回 `execution_state::active_character_changed`。相应[视图](../../executor/execution_view/active_character_changed.md)给出目标，牌桌仍可读取原出战角色；随后推进先完成设置，再处理变更响应。到达此现场前，已确认的费用响应、骰子支付及 [`dice_removed`](../events/dice_removed.md) 响应均已完成。

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

    static givm::program_entry<givm::character_initialization> handle(
        const definition_type&, const givm::character_view&,
        givm::character_initialization& event, const givm::table&, givm::random_fn&)
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
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::initialize_characters{ .player = givm::player_id{ 0 } }, givm::initialize_characters{ .player = givm::player_id{ 1 } }, givm::set_active_character{ .target = givm::character_id{ givm::player_id{ 0 }, 0 } }, givm::set_active_character{ .target = givm::character_id{ givm::player_id{ 1 }, 0 } }, givm::begin_action{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    const auto definition = ids.get_id<givm::character_view>("character");
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .characters = { definition } });
    table.load_deck(givm::player_id{ 1 }, givm::linked_deck{ .characters = { definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    auto state = execution.step(library, table, random);
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
双方结束声明次数: 2
```

## 参阅

| | |
| --- | --- |
| [`action_phase_started`](../events/action_phase_started.md) | 本回合行动阶段开始的通知 |
| [`before_action`](../events/before_action.md) | 当前行动玩家选择行动前的事件 |
| [`cost_of_switch`](../events/cost_of_switch.md) | 主动切换出战角色的费用计算事件 |
| [`round_end_declared`](../events/round_end_declared.md) | 玩家宣布本回合结束的通知 |
