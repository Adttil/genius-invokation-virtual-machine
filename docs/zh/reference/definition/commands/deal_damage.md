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

完成一组伤害。组内先依次计算每次伤害、扣除生命、处理击倒及元素附着；对局继续时，全部完成后再按伤害发生顺序调用反应后与伤害后的响应。

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

每次具体伤害依次经过 [`damage_calculation`](../events/damage_calculation.md)、[`damage_effect`](../events/damage_effect.md)、扣除生命、击倒处理及元素附着处理。计算响应结束后才按最终元素、目标及其附着确定反应；后续使用该次判定，不在扣血后重新判断。

扣血使角色生命降至零时，立即根据双方是否仍有存活角色判断胜负。濒死响应与复活流程尚未接入。若已终局，则直接结束，不再清理该角色的 attachment 与充能，也不再处理本次元素反应、剩余伤害或完成通知；若对局继续，才删除其全部 attachment、清空充能，再继续本组结算。已删除的 attachment 不参与后续广播。

初始描述按顺序处理。范围目标按其规定的角色顺序展开；反应派生伤害紧接引发它的伤害，先于原本排在后面的伤害。例如初始为 A、B，A 派生 a、b，实际扣血顺序是 A、a、b、B；派生伤害再次引起反应时沿用同一规则。

对局尚未结束时，所有实际伤害完成扣血、击倒处理和元素附着后，才按同样顺序进行完成通知：该次伤害有反应时先广播 [`after_elemental_reaction`](../events/after_elemental_reaction.md)，再广播 [`after_damage`](../events/after_damage.md)。此时才调用响应函数，响应读取到的牌桌已经包含本组全部扣血；通知中的伤害数据仍是对应伤害完成时的记录。

超导、感电默认使主目标伤害增加 1，并对其余存活角色分别造成 1 点穿透伤害；扩散对其余存活角色分别造成 1 点所扩散元素的伤害。这些派生伤害属于当前组。生成反应实体等其他默认反应后果尚未全部实现。

连续两个 `deal_damage` 分别结算，不自动合组。响应程序中执行的其他 `deal_damage` 也单独结算；独立命令若判定对局结束，会立即结束执行，外层尚未完成的处理不再继续。

计算倍率时向下取整，超出 uint32_t 范围时取其最大值；扣除生命不会低于 0。

## 观察

以 [`compile_mode::observed`](../../executor/compile_mode.md) 编译时，每次非零最终伤害扣除生命后先返回 `execution_state::health_reduced`，随后推进才处理击倒、终局判定及该次元素附着。到达某次扣血现场不表示本组已经完成；相应[视图](../../executor/execution_view/health_reduced.md)的伤害值不以目标原有生命为上限。

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
    givm::definition_source_library sources{};
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
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
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
| [`damage_calculation`](../events/damage_calculation.md) | 伤害计算事件 |
| [`damage_effect`](../events/damage_effect.md) | 扣除生命前的伤害结算事件 |
| [`after_damage`](../events/after_damage.md) | 伤害及其元素附着结算完成后的通知 |
