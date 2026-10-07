[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **deal_damage**

# givm::deal_damage

定义于头文件 `<givm/definition.hpp>`

造成伤害并执行相应元素反应。固定参数描述一次单体或范围伤害，动态参数允许按顺序提交多次伤害；[`end_segment`](end_segment.md) 决定哪些伤害归于同一段，[`settle`](settle.md) 决定何时处理通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `relative_character_target` | 固定来源，只允许单个角色 |
| `target` | `relative_character_target` | 固定目标及范围；默认偏移为 `INT32_MAX`，表示动态模式 |
| `value` | `std::uint32_t` | 基础伤害 |
| `multiplier_numerator` | `std::uint16_t` | 初始倍率分子，默认 1 |
| `multiplier_denominator` | `std::uint16_t` | 初始倍率分母，默认 1，不能为零 |
| `type` | `damage_type` | 伤害类型 |
| `flags` | `damage_flags` | 伤害属性 |

默认构造消费一个 [`deal_damage_input`](../command_inputs/deal_damage_input.md)，按数组顺序执行。固定模式把上述标量值复制到程序，不使用数组输入。每次单体或范围操作开始时采样目标，`others` 排除原始定位角色；范围内已确认击倒者被排除，中途复活或新增角色不进入这次范围。

普通目标不自动转移。同段生命为零但仍 `alive` 的角色仍完成伤害计算和反应；确认击倒后跳过。只有明确指定 `character_selection::prioritized`，才从指定位置开始按循环顺序选择第一名 `alive && health > 0` 的角色，不改变出战位置，也不请求选出战。

## 计算和反应

每个目标依次处理 `damage_preparation`、`damage_calculation`、所选反应定义的加伤、倍率、`damage_effect`，然后扣除生命。加值和倍率修饰使用同一次 `damage_calculation`；全部加值完成后只计算一次倍率，向上取整并饱和至 `UINT32_MAX`。倍率分子和分母可直接修改；叠加倍率时各效果向分子增加相应增量，向分母乘入相应因子。

反应槽位由元素组合确定，具体效果由目标对方的反应映射选择。`cancel_reaction_bonus` 取消所选反应的加伤响应。反应将发生时，`elemental_reaction_will_occur` 的 `new_aura` 已预填，可直接修改；`cancel_default_effects` 取消反应定义的后续效果。两个取消标记独立，反应事实仍进入通知。

扣血后立即执行本次元素附着及反应定义的后续程序，其产生的通知归于当前段。超导、感电与扩散的派生伤害保留原始来源；反应生成实体的归属由目标所在一方决定。后续命令读取已经更新的牌桌。

## 段收尾和通知

扣血不会立即广播濒死。段收尾时，对本段仍待处理且生命为零的存活角色广播 `character_will_be_defeated`；免于击倒效果使用 `healing_kind::prevent_defeat` 恢复生命。响应结束后仍为零才确认击倒、清空充能和附着。整段濒死处理完成后统一判断双方终局，支持同时全队击倒。

结算时先按创建顺序处理混合通知，再处理保留的入手通知，最后处理本段非致命及致命伤害摘要。每个目标只有一个 [`after_damage`](../events/after_damage.md)：伤害值累加，类型、属性和反应槽位取位或，不包含单次来源。致命摘要之前逐个清理角色附属，并完成自身离场和普通离场通知。

## 观察

观察模式在每次非零扣血之后返回 `health_reduced`，提供本击来源、伤害类型、反应和最终值；伤害值不以原有生命为上限。濒死与伤害后通知尚未处理。需要单次来源的观察者使用此视图，伤害后响应使用段摘要。

## 编译检查

`error_type` 为 `deal_damage_error`。检查动态根程序、来源和目标的玩家及范围、伤害类型、零倍率分母；错误的 `cause` 给出原因，`value` 给出相关字段值。

## 示例

```cpp
#include <utility>
#include <array>
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

int main()
{
    character_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    const std::array damages{
        givm::deal_damage{
            .source = givm::relative_character_target{ givm::relative_player::self, 0 },
            .target = givm::relative_character_target{ givm::relative_player::opponent, 0 },
            .value = 3, .type = givm::damage_type::physical, .flags = {} },
        givm::deal_damage{
            .source = givm::relative_character_target{ givm::relative_player::self, 0 },
            .target = givm::relative_character_target{ givm::relative_player::opponent, 0, givm::character_selection::others },
            .value = 1, .type = givm::damage_type::piercing, .flags = {} }
    };
    auto library_result = compile(
        sources, basics,
        std::tuple{ givm::select_active_character_both{}, damages[0], damages[1] },
        std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 0, .self_player = givm::player_id{ 0 } } };
    const auto definition = ids.get_id<givm::definition_category::character>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition } },
        givm::linked_deck{ .characters = { definition, definition } });
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    const givm::character_id other{ givm::player_id{ 1 }, 1 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 0 }, 0 });
    execution.view_in<givm::execution_state::remaining_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 1 }, 0 });
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
