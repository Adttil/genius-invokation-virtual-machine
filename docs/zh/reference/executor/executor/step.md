[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **step**

# givm::executor::step

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
execution_state step(
    const definition_library& library,
    table& card_table,
    TRandom& random_source
);
```
[`execution_state`](../execution_state.md)
[`table`](../../table/table.md)

推进对局，直到到达定义库所选编译模式需要报告的现场。普通模式返回输入现场或终局；观察模式还返回额外的领域观察现场。

使用 [`compile_mode::observed`](../compile_mode.md) 编译时，可观察现场包括扣除生命后的伤害、设置新出战角色前的切人，以及回合和行动机会的推进。一次调用可以跨过多项内部操作；它不生成持久日志。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 非 `const`、非 `volatile` 的对象类型；其左值须可无参数调用，且结果可隐式转换为 `std::uint32_t` |

## 参数

| | |
| --- | --- |
| `library` | 与当前执行现场配套的定义库；执行器不会在返回后持有它 |
| `card_table` | 与当前执行现场配套的牌桌，其中的定义 ID 须属于本次使用的定义库 |
| `random_source` | 本次推进使用的随机源，以左值传入；执行器不会在返回后持有它 |

## 返回值

本次到达的 [`execution_state`](../execution_state.md)。通过 [`view_in`](view_in.md) 取得相应视图；纯通知现场的信息直接从牌桌读取。

## 注意

每次推进须使用与建立当前现场时相同的编译产物。牌桌与执行器都不保存定义库指针；调用方负责保持程序现场、实体定义 ID 和所传定义库相匹配。

首次推进前须由 [`enter_entry`](enter_entry.md) 准备开始，或取得有效执行器的副本。`finished` 不能继续执行。

从任何输入现场继续推进前，都须通过相应视图提交合法输入。空选择也是输入，例如保留全部手牌或放弃剩余重投机会，仍须显式提交。查询、费用预览和合法性检查都不代替提交。

调用方可以自行保证输入合法，无须先调用检查接口。`step` 不检查输入是否已经提交，也不验证输入是否合法；违反此前提属于未定义行为。

先处理本次现场，再调用 `step` 继续。返回后可观察的数据借用当前现场，下一次推进会使先前取得的视图失效。

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
            .value = 999, .type = givm::damage_type::physical, .flags = {} }
    };
    const auto [library, ids] = compile(
        sources,
        std::tuple{
            givm::select_active_character_both{},
            givm::set_active_character{ .target = givm::relative_character_target{ givm::relative_player::self, 1 } },
            givm::deal_damage{ .damages = damages } },
        std::tuple{}, givm::compile_mode::observed);
    givm::table table{ { .max_rounds = 0, .self_player = givm::player_id{ 0 } } };
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition, definition } },
        givm::linked_deck{ .characters = { definition } });
    const givm::character_id original{ givm::player_id{ 0 }, 0 };
    const givm::character_id attacker{ givm::player_id{ 0 }, 1 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
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
    execution.step(library, table, random);
    const auto switch_view = execution.view_in<givm::execution_state::active_character_changed>();
    std::println("切人现场指向新角色: {}", switch_view.character() == attacker);
    std::println("牌桌仍为原出战角色: {}", table[switch_view.character().player_id].state().active_character == original);
    const auto state = execution.step(library, table, random);
    std::println("切人后到达伤害现场: {}", state == givm::execution_state::health_reduced);
    std::println("新出战角色已写入牌桌: {}", table[givm::player_id{ 0 }].state().active_character == attacker);
    const auto view = execution.view_in<givm::execution_state::health_reduced>();
    std::println("本次伤害: {}", view.value());
    std::println("剩余生命: {}", table[target].state().health);
    std::println("继续推进至终局: {}", execution.step(library, table, random) == givm::execution_state::finished);
}
```

输出

```text
切人现场指向新角色: true
牌桌仍为原出战角色: true
切人后到达伤害现场: true
新出战角色已写入牌桌: true
本次伤害: 999
剩余生命: 0
继续推进至终局: true
```

## 参阅

| | |
| --- | --- |
| [`compile_mode`](../compile_mode.md) | 选择需要报告的现场 |
| [`execution_view`](../execution_view.md) | 执行现场视图 |
