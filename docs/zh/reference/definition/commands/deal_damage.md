[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **deal_damage**

# givm::deal_damage

定义于头文件 `<givm/definition.hpp>`

```cpp
struct deal_damage
{
    using input_type = deal_damage_input;

    std::span<const fixed_damage> damages{};
};
```

完成一组伤害。先为整组确定属性、元素反应及派生伤害，并推进元素附着；再逐段计算数值、扣除生命、处理击倒与默认反应实体生成；最后处理超载切人及反应后、伤害后响应。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`deal_damage_input`](../command_inputs/deal_damage_input.md)，动态模式下的输入类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `damages` | `std::span<const fixed_damage>` | 编译时提供的固定伤害描述；非空时不消费响应输入 |

## 输入

默认构造 `deal_damage{}` 消费响应通过 `invoke` 提交的一个 [`deal_damage_input`](../command_inputs/deal_damage_input.md)。其 `damages` 包含本次全部 [`damage`](../command_inputs/damage.md) 描述，数量可在响应时决定；空数组不产生伤害。这些描述属于同一组，按数组顺序处理。`invoke` 复制数组内容，返回后不再借用原数组。

非空 `damages` 的内容在编译该命令时复制，所引用的数组只需保持有效至相应 `add_program` 或整体 `compile` 返回。执行时不借用原数组。

[`fixed_damage`](fixed_damage.md) 以位置描述来源和目标，不保存对局实体 ID。先以当前效果的本方或对方确定玩家，再在准备这一条初始描述时解析出战位置，因此能表达出战角色、下一个角色、上一个角色，以及定位角色以外的其他角色或全部角色。作用范围由 [`character_selection`](character_selection.md) 指定。本方取自 [`table_state::self_player`](../../table/table_state.md)，根流程使用相对描述时须显式设置有效本方。需要保留具体技能、召唤物或卡牌来源时，使用动态 `damage` 输入；固定命令不会把来源隐式设为响应实体。

初始描述数量不等于实际伤害次数：一条范围描述可以命中多个角色，元素反应还可能派生新的伤害。

## 结算

一组伤害分为准备、数值结算、完成通知三个阶段。准备阶段先按初始描述顺序处理 [`damage_preparation`](../events/damage_preparation.md)。每条初始描述只广播一次属性修饰；范围描述以解析出的角色为锚点，按修饰后的目标展开，并将同一份来源、元素、标志、基础数值与倍率用于所有命中。默认反应派生伤害不再广播属性修饰。

每个具体命中均按其元素和目标当时的完整附着判定反应，分别保存为 `reaction` 与 `reacted_aura`。有反应时先广播 [`elemental_reaction_will_occur`](../events/elemental_reaction_will_occur.md)，选择 `replacement_reaction`；后写标签无条件覆盖前写，空标签表示默认效果。随后立即推进元素附着，并准备默认反应派生伤害。准备期间同样会完整执行响应返回的程序。

整组准备完成后，才依次广播 [`damage_calculation`](../events/damage_calculation.md) 和 [`damage_effect`](../events/damage_effect.md)，计算数值、处理减伤和护盾、扣除生命并处理击倒。数值阶段读取牌桌时，本组后续命中的元素附着也已经推进；若效果判断的是本次伤害原先面对的附着，应读取 `damage_calculation::reacted_aura`，不能用牌桌上的当前附着代替。例如“对已附着火元素的角色增伤”不会因这次火伤刚附上火而满足条件。

原始反应、原附着与替代标签确定后不再重算。某目标若在数值结算轮到自己之前已经死亡，本次对它的伤害跳过，不发送该次反应后与伤害后通知；其准备阶段已经产生的其他目标伤害仍会继续。死亡处理清除的附着也不会被后续伤害重新写回。准备阶段和数值阶段均不是牌桌快照：各自执行的响应程序仍能影响随后读取的运行状态。

数值响应结束后，仅在替代标签为空时加入默认反应加伤，再应用倍率，最后进入减伤与护盾阶段。非空标签同时取消默认派生伤害、实体生成和超载切人；附着消耗始终按原始反应处理。替代反应仍被视为发生了原始反应，响应者可在后续已有事件中根据标签完成替代效果。

扣血使角色生命降至零时，先全局广播 [`character_will_be_defeated`](../events/character_will_be_defeated.md)。濒死角色的技能和尚未清除的附属仍可响应，响应提交的治疗程序可以恢复其生命。全部响应及其程序结束后，以角色的实际生命为准：若已经恢复至非零，保留附属和充能；若仍为零，才根据双方是否仍有存活角色判断胜负。若已终局，则直接结束，不再清理该角色的 attachment 与充能，也不再处理剩余默认反应效果、剩余伤害或完成通知；准备阶段已经完成的附着不回退。若对局继续，才删除其全部 attachment、清空充能与元素附着，再继续本组结算。已删除的 attachment 不参与后续广播。

初始描述按顺序准备。范围目标按其规定的角色顺序展开；反应派生伤害紧接引发它的伤害，先于原本排在后面的伤害。例如初始为 A、B，A 派生 a、b，实际扣血顺序是 A、a、b、B；派生伤害再次引起反应时沿用同一规则。

准备完成且对局尚未结束时，每段伤害先完成扣血、击倒处理及默认反应实体生成，再继续下一段伤害；整组伤害结束后先完成已登记的超载切人及其通知，才开始完成通知。轮到某次伤害时，有反应则先广播 [`after_elemental_reaction`](../events/after_elemental_reaction.md)，再广播 [`after_damage`](../events/after_damage.md)。此时才调用响应函数，响应读取的是本组结算后的当前牌桌；通知中的伤害数据仍是对应伤害完成时的记录。

超导、感电默认使主目标伤害增加 1，并对准备时的其余存活角色分别造成 1 点穿透伤害；扩散对这些角色分别造成 1 点所扩散元素的伤害。这些派生伤害属于当前组，保留各自的反应判定、替代标签选择、数值计算和完成通知，但不再次附魔。

超载默认增加 2 点伤害。反应判定及标签选择完成时，若本次没有替代标签且目标是其所属玩家的出战角色，就为该玩家登记一次强制切换；同一伤害组只采用首次符合条件的超载。整组全部伤害与逐段反应处理结束后，以该玩家当时的出战位置为起点，按角色顺序循环寻找下一个存活角色，切换并完成 [`active_character_changed`](../events/active_character_changed.md) 通知，再进行反应后和伤害后通知。原目标后来死亡或出战角色中途变化，不撤销已登记的切换；若唯一存活角色已出战，则不切换也不通知。判定终局时不再执行尚未完成的切换。

激化、燃烧、绽放各自提供 1 点默认反应加伤，并分别在发生反应角色的对方请求生成激化领域、燃烧烈焰、草原核。归属由反应目标决定，与伤害来源和当前行动玩家无关。实体定义由定义库指定；已有同定义实体时按其重复生成响应处理，燃烧烈焰还遵守召唤物容量限制。

组末执行超载时，若该玩家当时的出战角色具有 `control_immunity` 附属，则取消此次切换，不产生切人通知或观察现场。检查依据是组末当前的保护，已确定的超载加伤和附着处理不受影响。

冻结默认增加 1 点伤害，并在本段扣血与击倒处理后向仍存活的目标施加定义库指定的冻结附属。目标已死亡时不再添加；目标具有免控保护时，控制附属的施加被阻止，反应与默认加伤仍保留。随库提供的冻结规则见[基础定义源](../../basic_definitions.md#冻结与控制)。

默认实体在引发反应的本段伤害扣血并通过终局判定后生成，不会追溯改变本段伤害；后续同组伤害和伤害后响应另行造成的伤害，可以使用仍在场且满足条件的实体。激化领域只增强对敌方出战角色的雷、草伤害，对后台角色的伤害不满足条件。重复生成响应若提交程序，先完成该程序，再继续下一段伤害。每次完成通知仍按自己的广播规则确定候选响应者。其他默认反应后果尚未全部实现。

连续两个 `deal_damage` 分别结算，不自动合组。响应程序中执行的其他 `deal_damage` 也单独结算；独立命令若判定对局结束，会立即结束执行，外层尚未完成的处理不再继续。

计算倍率时向下取整，超出 uint32_t 范围时取其最大值；扣除生命不会低于 0。

## 观察

以 [`compile_mode::observed`](../../executor/compile_mode.md) 编译时，每次非零最终伤害扣除生命后先返回 `execution_state::health_reduced`，随后推进才广播濒死、处理击倒和终局判定及默认反应实体生成。本组的元素附着已在准备阶段推进。观察当前段扣血时，可以看到前段已经生成且仍在场的实体，当前段的默认实体尚未生成。到达某次扣血现场不表示本组已经完成；相应[视图](../../executor/execution_view/health_reduced.md)的伤害值不以目标原有生命为上限。

该视图同时提供原始反应与替代标签。组末超载确实改变出战角色时，还会到达 `execution_state::active_character_changed` 现场；其观察与切人通知都先于本组反应后和伤害后通知。

## 示例

```cpp
#include <array>
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

int main()
{
    character_source source{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const std::array damages{
        givm::fixed_damage{
            .source = givm::relative_character_target{ givm::relative_player::self, 0 },
            .target = givm::relative_character_target{ givm::relative_player::opponent, 0 },
            .value = 3, .type = givm::damage_type::physical, .flags = {} },
        givm::fixed_damage{
            .source = givm::relative_character_target{ givm::relative_player::self, 0 },
            .target = givm::relative_character_target{ givm::relative_player::opponent, 0, givm::character_selection::others },
            .value = 1, .type = givm::damage_type::piercing, .flags = {} }
    };
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::select_active_character_both{}, givm::deal_damage{ .damages = damages } },
        std::tuple{}, givm::compile_mode::normal);
    givm::table table{ { .max_rounds = 0, .self_player = givm::player_id{ 0 } } };
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition } },
        givm::linked_deck{ .characters = { definition, definition } });
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    const givm::character_id other{ givm::player_id{ 1 }, 1 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(
        givm::character_id{ givm::player_id{ 0 }, 0 });
    execution.step(library, table, random);
    execution.view_in<givm::execution_state::remaining_active_character_selection>().select(
        givm::character_id{ givm::player_id{ 1 }, 0 });
    execution.step(library, table, random);
    std::println("目标剩余生命: {}", table[target].state().health);
    std::println("其余角色剩余生命: {}", table[other].state().health);
}
```

输出

```text
目标剩余生命: 7
其余角色剩余生命: 9
```

## 参阅

| | |
| --- | --- |
| [`damage_preparation`](../events/damage_preparation.md) | 伤害属性修饰事件 |
| [`damage_calculation`](../events/damage_calculation.md) | 伤害计算事件 |
| [`damage_effect`](../events/damage_effect.md) | 扣除生命前的伤害结算事件 |
| [`after_damage`](../events/after_damage.md) | 伤害及其元素附着结算完成后的通知 |
