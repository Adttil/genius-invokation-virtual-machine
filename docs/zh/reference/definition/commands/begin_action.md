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
| [`error_type`](#编译检查) | `begin_action_error` 的别名，即本命令的编译检查错误类型 |

## 编译检查

```cpp
enum class begin_action_error {};
```

`begin_action::error_type` 是 `givm::begin_action_error` 的别名。这是没有枚举项的空枚举类型，本命令没有编译期参数错误。

## 注意

先发出 [`action_phase_started`](../events/action_phase_started.md)，每次选择行动前发出 [`before_action`](../events/before_action.md)。支持使用技能或特技、打出手牌、元素调和、主动切换出战角色和宣布结束；当前行动方必须已有出战角色。双方均宣布结束后，本命令才结束行动阶段。

每次 `before_action` 及其响应程序完成后、建立行动候选前，检查出战角色的准备技能附属；快速行动后再次选择时也会检查。若角色未受控制，按附属遍历顺序选中第一个支持 [`this_prepared_skill_use`](../events/this_prepared_skill_use.md) 的有效实体，自动执行本次准备技能，不返回 `action_selection`。若受控制，则保留准备技能附属，正常进入行动选择。

准备技能先离场并完成离场通知，再调用其自身效果；选定后不因离场响应改变状态而撤销。它无需支付，不发送普通技能或特技使用通知；默认战斗行动，可由响应改为快速行动，最终速度决定是否交接行动权和消耗 `can_plunge`。一次只消耗一个准备技能；跨回合或宣布结束本身不清除准备技能附属。

等待选择行动时，执行器返回 `execution_state::action_selection`，通过相应的[现场视图](../../executor/execution_view/action_selection.md)预览费用、检查或选择行动。出牌选择当前行动玩家的有效手牌，并提供支付骰子及至多两个目标；目标参数默认为空 span，超过两个的元素忽略。目标和用牌条件由牌定义决定。主动切换选择当前行动玩家存活、非出战的角色。调用方保证支付满足费用、骰子持有数量及出战角色充能；宣布结束无需支付骰子。

调用方必须通过 `use_skill`、`use_technique`、`play_card`、`elemental_tuning`、`switch_active_character` 或 `declare_round_end` 提交本次行动输入并立即推进。费用预览、支付检查与目标检查不提供行动输入；等待玩家决定期间由上层保留当前现场。

技能或特技选择前由调用方通过现场的 [`is_controlled`](../../executor/execution_view/action_selection/is_controlled.md) 检查控制状态，执行器不自动拒绝受控角色使用技能或特技。主动切换不受控制或免控附属限制。需要限制受控角色使用的卡牌，由牌定义在目标与用牌条件查询中检查。

技能、出牌与切换分别使用从零开始的候选索引。技能通过 [`skill_count`](../../executor/execution_view/action_selection/skill_count.md) 查询数量、[`skill_id`](../../executor/execution_view/action_selection/skill_id.md) 查询对应技能 ID。通过 [`card_count`](../../executor/execution_view/action_selection/card_count.md) 和 [`switch_target_count`](../../executor/execution_view/action_selection/switch_target_count.md) 查询数量，通过 [`card_id`](../../executor/execution_view/action_selection/card_id.md) 和 [`switch_target`](../../executor/execution_view/action_selection/switch_target.md) 查询对应实体 ID；技能和牌的效果目标仍使用 ID。

技能候选仅包含出战角色中支持 [`this_skill_use`](../events/this_skill_use.md) 的有效技能。通过 [`calculate_skill_cost`](../../executor/execution_view/action_selection/calculate_skill_cost.md) 查询费用，按需独立进行 [`skill_payment_validate`](../../executor/execution_view/action_selection/skill_payment_validate.md) 和 [`skill_targets_validate`](../../executor/execution_view/action_selection/skill_targets_validate.md)，再通过 [`use_skill`](../../executor/execution_view/action_selection/use_skill.md) 提交技能、骰子及目标。支付后广播 [`skill_will_be_used`](../events/skill_will_be_used.md)，未取消时执行技能自身效果，之后均广播 [`skill_used`](../events/skill_used.md)；取消效果不撤销本次使用或支付。行动速度采用生效前响应的最终结果。

特技来自出战角色的特技装备，至多一个，需支持 [`this_technique_use`](../events/this_technique_use.md)。通过 [`has_technique`](../../executor/execution_view/action_selection/has_technique.md) 查询是否存在主动特技，再由 [`calculate_technique_cost`](../../executor/execution_view/action_selection/calculate_technique_cost.md) 报价，按需独立检查支付和目标，最后用 [`use_technique`](../../executor/execution_view/action_selection/use_technique.md) 提交。特技也可消耗充能；支付后依次处理特技使用前广播、自身效果及使用后通知，取消原效果仍保留使用后通知。

通过 [`calculate_card_cost`](../../executor/execution_view/action_selection/calculate_card_cost.md) 同步计算出牌费用，通过 [`card_payment_validate`](../../executor/execution_view/action_selection/card_payment_validate.md) 与 [`card_targets_validate`](../../executor/execution_view/action_selection/card_targets_validate.md) 分别检查支付及用牌条件。目标检查按 span 中的目标数量分步进行，允许检查空选择，告知当前选择是否有效、能否完成或继续；检查第二目标时可假设第一目标合法。两项检查相互独立，由调用方按需使用。目标检查通过 [`card_target_validation`](../queries/card_target_validation.md) 返回结果，不接收随机源。费用响应不得使用随机数，调用随机函数属于未定义行为。

[`play_card`](../../executor/execution_view/action_selection/play_card.md) 同步报价并提交，`play_card_with_cached_cost` 使用已有报价；Debug 报价时检查完整目标，提交时检查支付，Release 不检查。提交立即推进，先让牌离手，再逐个执行并完整结算已确认的费用效果，随后饱和扣除骰子与原付费角色的充能，并将实际变化通知登记到行动段。之后广播 [`card_will_be_played`](../events/card_will_be_played.md)，未被反制时执行本牌的 [`this_card_play`](../events/this_card_play.md)，并登记 [`card_played`](../events/card_played.md)。付款通知和行动通知按所属段结算。反制只取消原效果，不退还费用或撤销离手。最后按报价确定的行动速度保留或交接行动权。

通过 [`calculate_switch_cost`](../../executor/execution_view/action_selection/calculate_switch_cost.md) 可以同步预览切换至指定角色的费用，无需推进执行器或传入随机源。费用响应不得使用随机数，调用随机函数属于未定义行为；目标为只读。完整报价后可调用 [`switch_payment_validate`](../../executor/execution_view/action_selection/switch_payment_validate.md)，依次检查骰子是否匹配费用、持有数量是否足够、非零充能费用的类型是否匹配及出战角色充能是否足够。

通过 [`switch_active_character`](../../executor/execution_view/action_selection/switch_active_character.md) 同步报价并提交切换，或通过 [`switch_active_character_with_cached_cost`](../../executor/execution_view/action_selection/switch_active_character_with_cached_cost.md) 使用已有报价标识。两者均立即推进，执行已确认的费用效果、支付及切换。Debug 提交检查支付与报价状态，Release 由调用方保证合法。同一行动窗口内每个切换目标只允许报价一次，可以通过标识反复读取结果。

成功切换时，原出战角色上的所有准备技能附属一起标记为离场，按顺序逐个完成 [`attachment_removed`](../events/attachment_removed.md) 通知后，才处理正常的切换通知。

在 [`compile_mode::observed`](../../executor/compile_mode.md) 模式下推进主动切人时，在写入新出战角色之前返回 `execution_state::active_character_changed`。相应[视图](../../executor/execution_view/active_character_changed.md)给出目标，牌桌仍可读取切换前的出战角色及其准备技能附属；随后推进才实际切换、清除这些附属并处理通知。到达此现场前，费用效果及其派生结算已经完成，骰子与充能已经实际扣除；付款通知仍归入行动段，随后与切换通知一起结算。

以 [`compile_mode::observed`](../../executor/compile_mode.md) 编译时，每次新的行动机会先返回 `execution_state::action_started`，随后才处理 `before_action`。快速行动不结束当前机会；战斗行动结束后，即使另一方已经宣布结束、仍由当前玩家行动，也会报告新的行动机会。宣布结束时先返回 `execution_state::round_end_declared`，此时牌桌上的 `active_player` 仍是宣布结束的一方，随后推进才处理其结束响应。

## 示例

```cpp
#include <utility>
#include <array>
#include <cstddef>
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct character_source
{
    static constexpr auto category = givm::definition_category::character;
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
    static constexpr auto category = givm::definition_category::card;
    struct definition_type {};
    std::string_view name() const { return "card"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    character_source source{};
    card_source card{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    if(not sources.add(card)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{ givm::select_active_character_both{}, givm::draw_cards{ .position = 0, .count = 1 }, givm::begin_action{} },
        std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 0, .self_player = givm::player_id{ 0 } } };
    const auto definition = ids.get_id<givm::definition_category::character>("character");
    const auto card_definition = ids.get_id<givm::definition_category::card>("card");
    load_deck(table, library,
        givm::linked_deck{
            .cards = { card_definition }, .characters = { definition, definition }
        },
        givm::linked_deck{ .characters = { definition } });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 0 }, 0 });
    auto state = execution.view_in<givm::execution_state::remaining_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 1 }, 0 });
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
        state = execution.view_in<givm::execution_state::action_selection>().declare_round_end(library, table, random);
        ++declarations;
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
| [`this_prepared_skill_use`](../events/this_prepared_skill_use.md) | 代替选择而自动执行的准备技能 |
| [`cost_of_switch`](../events/cost_of_switch.md) | 主动切换出战角色的费用计算事件 |
| [`round_end_declared`](../events/round_end_declared.md) | 玩家宣布本回合结束的通知 |

费用报价接收完整目标并返回对应的强类型标识；采用缓存只接收标识与支付骰子，不重新指定来源或目标。费用效果及全部连锁逐个完成后，按原选各色数量饱和付款，其他骰色或万能骰不补足。充能锁定确认时的原付费角色，费用效果切人不改变付款对象。付款通知报告实际变化并归入实际行动段；费用效果终局后停止后续付款和行动。
