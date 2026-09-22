[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **deal_damage**

# givm::deal_damage

定义于头文件 `<givm/definition.hpp>`

```cpp
struct deal_damage
{
    std::span<const damage> damages{};
    std::size_t input_count = 1;
};
```

完成一组伤害。组内逐段完成伤害计算、扣除生命、击倒、元素附着及默认反应实体生成；整组结束后处理超载切人，再按伤害发生顺序调用反应后与伤害后的响应。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `damages` | `std::span<const damage>` | 编译时提供的初始伤害描述；非空时使用固定参数，不消费响应输入 |
| `input_count` | `std::size_t` | `damages` 为空时消费的动态伤害描述数量，默认为 1 |

## 输入

默认构造 `deal_damage{}` 消费响应通过 `invoke` 提交的一个 [`damage`](../events/damage.md)。`input_count` 必须大于零；指定 `input_count = 2` 时，响应依次提交两个 `damage`，例如 `return context.invoke(entry, first, second);`。这些描述属于同一组，按提交顺序处理。

非空 `damages` 的内容在编译该命令时复制，所引用的数组只需保持有效至相应 `add_program` 或整体 `compile` 返回。执行时不借用原数组；此时 `input_count` 不参与结算。

初始描述数量不等于实际伤害次数：一条范围描述可以命中多个角色，元素反应还可能派生新的伤害。

## 结算

每次具体伤害先完成 [`damage_preparation`](../events/damage_preparation.md) 的属性修饰，判定反应并选择替代标签，再进入 [`damage_calculation`](../events/damage_calculation.md) 的数值计算，随后经过 [`damage_effect`](../events/damage_effect.md) 的减伤与护盾处理、扣除生命、击倒处理及元素附着处理。附魔等属性修饰全部结束后，数值响应读取的元素、来源、目标与伤害标志均已确定。

元素反应在属性修饰结束后、数值计算开始前判定一次：使用最终元素、目标和目标当时的完整附着，分别作为 `reaction` 与 `reacted_aura` 供后续响应读取。有反应时先广播 [`elemental_reaction_will_occur`](../events/elemental_reaction_will_occur.md)，由响应者选择 `replacement_reaction`；后写标签无条件覆盖前写，空标签表示默认效果。之后即使响应效果改变目标附着，也不重新判定本次反应；数值计算、伤害效果及完成通知携带同一个原始 `reaction` 和只读替代标签。

数值响应结束后，仅在替代标签为空时加入默认反应加伤，再应用倍率，最后进入减伤与护盾阶段。非空标签同时取消默认派生伤害、实体生成和超载切人；附着消耗始终按原始反应处理。替代反应仍被视为发生了原始反应，响应者可在后续已有事件中根据标签完成替代效果。

扣血使角色生命降至零时，立即根据双方是否仍有存活角色判断胜负。濒死响应与复活流程尚未接入。若已终局，则直接结束，不再清理该角色的 attachment 与充能，也不再处理本次元素附着、剩余默认反应效果、剩余伤害或完成通知；若对局继续，才删除其全部 attachment、清空充能，再继续本组结算。已删除的 attachment 不参与后续广播。

初始描述按顺序处理。范围目标按其规定的角色顺序展开；反应派生伤害紧接引发它的伤害，先于原本排在后面的伤害。例如初始为 A、B，A 派生 a、b，实际扣血顺序是 A、a、b、B；派生伤害再次引起反应时沿用同一规则。

对局尚未结束时，每段伤害先完成扣血、击倒处理、元素附着及默认反应实体生成，再继续下一段伤害；整组伤害结束后先完成已登记的超载切人及其通知，才开始完成通知。轮到某次伤害时，有反应则先广播 [`after_elemental_reaction`](../events/after_elemental_reaction.md)，再广播 [`after_damage`](../events/after_damage.md)。此时才调用响应函数，响应读取的是本组结算后的当前牌桌；通知中的伤害数据仍是对应伤害完成时的记录。

超导、感电默认使主目标伤害增加 1，并对其余存活角色分别造成 1 点穿透伤害；扩散对其余存活角色分别造成 1 点所扩散元素的伤害。这些派生伤害属于当前组。

超载默认增加 2 点伤害。反应判定及标签选择完成时，若本次没有替代标签且目标是其所属玩家的出战角色，就为该玩家登记一次强制切换；同一伤害组只采用首次符合条件的超载。整组全部伤害与逐段反应处理结束后，以该玩家当时的出战位置为起点，按角色顺序循环寻找下一个存活角色，切换并完成 [`active_character_changed`](../events/active_character_changed.md) 通知，再进行反应后和伤害后通知。原目标后来死亡或出战角色中途变化，不撤销已登记的切换；若唯一存活角色已出战，则不切换也不通知。判定终局时不再执行尚未完成的切换。

激化、燃烧、绽放各自提供 1 点默认反应加伤，并分别在发生反应角色的对方请求生成激化领域、燃烧烈焰、草原核。归属由反应目标决定，与伤害来源和当前行动玩家无关。实体定义由定义库指定；已有同定义实体时按其重复生成响应处理，燃烧烈焰还遵守召唤物容量限制。

组末执行超载时，若该玩家当时的出战角色具有 `control_immunity` 附属，则取消此次切换，不产生切人通知或观察现场。检查依据是组末当前的保护，已确定的超载加伤和附着处理不受影响。

冻结默认增加 1 点伤害，并在本段扣血、击倒与附着处理后向仍存活的目标施加定义库指定的冻结附属。目标已死亡时不再添加；目标具有免控保护时，控制附属的施加被阻止，反应与默认加伤仍保留。随库提供的冻结规则见[基础定义源](../../basic_definitions.md#冻结与控制)。

默认实体在引发反应的本段伤害扣血并通过终局判定后生成，不会追溯改变本段伤害；后续同组伤害和伤害后响应另行造成的伤害，可以使用仍在场且满足条件的实体。激化领域只增强对敌方出战角色的雷、草伤害，对后台角色的伤害不满足条件。重复生成响应若提交程序，先完成该程序，再继续下一段伤害。每次完成通知仍按自己的广播规则确定候选响应者。其他默认反应后果尚未全部实现。

连续两个 `deal_damage` 分别结算，不自动合组。响应程序中执行的其他 `deal_damage` 也单独结算；独立命令若判定对局结束，会立即结束执行，外层尚未完成的处理不再继续。

计算倍率时向下取整，超出 uint32_t 范围时取其最大值；扣除生命不会低于 0。

## 观察

以 [`compile_mode::observed`](../../executor/compile_mode.md) 编译时，每次非零最终伤害扣除生命后先返回 `execution_state::health_reduced`，随后推进才处理击倒、终局判定、该次元素附着及默认反应实体生成。观察当前段扣血时，可以看到前段已经生成且仍在场的实体，当前段的默认实体尚未生成。到达某次扣血现场不表示本组已经完成；相应[视图](../../executor/execution_view/health_reduced.md)的伤害值不以目标原有生命为上限。

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
        givm::damage{
            .source = givm::character_id{ givm::player_id{ 0 }, 0 },
            .target = givm::character_id{ givm::player_id{ 1 }, 0 },
            .value = 3, .type = givm::damage_type::physical, .flags = {} },
        givm::damage{
            .source = givm::character_id{ givm::player_id{ 0 }, 0 },
            .target = givm::other_characters_target{ givm::character_id{ givm::player_id{ 1 }, 0 } },
            .value = 1, .type = givm::damage_type::piercing, .flags = {} }
    };
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::deal_damage{ .damages = damages } },
        std::tuple{}, givm::compile_mode::normal);
    givm::table table{ { .max_rounds = 0 } };
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
