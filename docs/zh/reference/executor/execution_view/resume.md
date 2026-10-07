[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **resume**

# givm::execution_view<State>::resume

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class TRandom>
execution_state resume(
    this const auto& self, const definition_library& library, table& card_table, TRandom& random
);
```

从初始化完成或观察现场继续对局，直到到达编译模式要求报告的下一处现场。仅 `initialized` 和观察现场提供本函数；输入现场通过提交输入推进，`finished` 不提供推进操作。

数据观察视图的同名成员采用普通 `const` 成员函数形式，参数与行为相同。

## 模板参数

| | |
| --- | --- |
| `self` 的推导类型 | 与当前现场种类对应的视图类型。 |
| `TRandom` | 非 `const`、非 `volatile` 的可调用对象类型，其无参数调用结果可隐式转换为 `std::uint32_t`。 |

## 参数

| | |
| --- | --- |
| `library` | 与当前执行现场配套的定义库。 |
| `card_table` | 与当前执行现场配套的牌桌。 |
| `self` | 当前初始化或观察现场的视图。 |
| `random` | 本次推进使用的随机源，以左值传入；执行器不会在返回后持有它。 |

## 返回值

本次到达的 [`execution_state`](../execution_state.md)。普通模式返回输入现场或终局；观察模式还返回观察现场。通过 [`executor::view_in`](../executor/view_in.md) 取得对应视图。

## 异常

未定义 `NDEBUG` 时，现场种类不匹配或视图已经失效会抛出 [`execution_view_error`](../execution_view_error.md)。执行过程中的 [`program_input_error`](../program_input_error.md)、[`command_input_error`](../command_input_error.md)、[`history_access_error`](../history_access_error.md)，以及定义源或随机源抛出的异常，均向外传播。

开始推进后不提供异常回滚，旧视图已经失效；若执行过程中抛出异常，不应继续使用原执行现场。

## 注意

初始化现场由 [`executor::start`](../executor/start.md) 返回；初始化完成但尚未执行命令时，可以检查或复制牌桌与执行器。执行器副本应重新取得自己的 `initialized` 视图。

每次推进使用与建立当前现场时配套的编译产物和牌桌。开始推进会使全部旧视图以及从它们借用的引用、span 失效，即使返回的现场种类仍相同。

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
            .value = 999, .type = givm::damage_type::physical, .flags = {} }
    };
    auto library_result = compile(
        sources, basics,
        std::tuple{
            givm::select_active_character_both{},
            givm::set_active_character{ .target = givm::relative_character_target{ givm::relative_player::self, 1 } },
            damages[0], givm::settle{} },
        std::tuple{}, givm::compile_mode::observed);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 0, .self_player = givm::player_id{ 0 } } };
    const auto definition = ids.get_id<givm::definition_category::character>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition, definition } },
        givm::linked_deck{ .characters = { definition } });
    const givm::character_id original{ givm::player_id{ 0 }, 0 };
    const givm::character_id attacker{ givm::player_id{ 0 }, 1 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 0 }, 0 });
    execution.view_in<givm::execution_state::remaining_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 1 }, 0 });
    execution.view_in<givm::execution_state::initial_active_characters_selected>().resume(library, table, random);
    const auto switch_view = execution.view_in<givm::execution_state::active_character_changed>();
    std::println("切人现场指向新角色: {}", switch_view.character() == attacker);
    std::println("牌桌仍为原出战角色: {}", table[switch_view.character().player_id()].state().active_character == original);
    const auto state = switch_view.resume(library, table, random);
    std::println("切人后到达伤害现场: {}", state == givm::execution_state::health_reduced);
    std::println("新出战角色已写入牌桌: {}", table[givm::player_id{ 0 }].state().active_character == attacker);
    const auto view = execution.view_in<givm::execution_state::health_reduced>();
    std::println("本次伤害: {}", view.value());
    std::println("剩余生命: {}", table[target].state().health);
    std::println("继续推进至终局: {}", view.resume(library, table, random) == givm::execution_state::finished);
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
| [`compile_mode`](../compile_mode.md) | 决定需要报告哪些执行现场的编译模式 |
| [`executor::start`](../executor/start.md) | 初始化执行器与历史摘要 |
