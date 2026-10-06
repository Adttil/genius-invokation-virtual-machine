[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **is_controlled**

# givm::definition_library::is_controlled

定义于头文件 `<givm/runtime.hpp>`

```cpp
bool is_controlled(character_view character) const noexcept;
```

检查角色当前是否处于冻结等控制状态，供行动界面和定义规则判断技能或主动特技能否使用。

## 参数

| | |
| --- | --- |
| `character` | 与本定义库配套牌桌中的有效角色 |

## 返回值

角色当前仍在场的附属中，存在定义带 `control` 标签的实体时返回 `true`，否则返回 `false`。

## 注意

查询读取当前附属；删除一种控制后，若另一种控制仍在场，仍返回 `true`。附属状态值为零不会自行解除控制。免控保护也不会解除已经存在的控制。

本函数只提供判断，不提交或阻止行动。调用方应在选择技能前检查控制；[`use_skill`](../execution_view/action_selection/use_skill.md) 不自动进行此检查。需要在用牌时限制受控角色的卡牌，可在 [`card_target_validation`](../../definition/queries/card_target_validation.md) 中调用本函数。

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
    using definition_category = givm::character_view;
    struct definition_type {};
    std::string_view name() const { return "character"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
    }
};

struct effect_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "freeze"; }
    auto attachment_dependencies() const { return std::array{ givm::genshin_impact::frozen_3_3_0.name() }; }

    givm::program_entry compile(givm::definition_compile_context& context) const
    {
        return context.add_program(givm::attach{ .definition = context.resolve_id<givm::attachment_view>(givm::genshin_impact::frozen_3_3_0.name()) });
    }

    static givm::program_entry handle(const givm::program_entry& entry,
        givm::battle_started&, givm::handle_context<givm::deck_card_view>& context, std::uint32_t = 0)
    {
        return context.invoke(entry);
    }
};

int main()
{
    character_source character{};
    const effect_source effect{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(character, effect)) return 1;
    const givm::character_id target{ givm::player_id{ 0 }, 0 };
    auto library_result = compile(sources, basics,
        std::tuple{ givm::select_active_character_both{} },
        std::tuple{
            givm::start_battle{}, givm::settle{},
            givm::start_dice_roll_phase{ .count = 1 },
            givm::start_round{}, givm::settle{}, givm::end_game{ .result = givm::game_result::both_loss }
        }, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 2, .self_player = givm::player_id{ 0 } } };
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .cards = { ids.get_id<givm::card_definition>("freeze") }, .characters = { definition } },
        givm::linked_deck{ .characters = { definition } });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 0 }, 0 });
    execution.view_in<givm::execution_state::remaining_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 1 }, 0 });
    std::println("投骰时出战角色受控: {}", library.is_controlled(table[target]));
    std::println("冻结属于控制: {}", library.is_control(ids.get_id<givm::attachment_view>(givm::genshin_impact::frozen_3_3_0.name())));
    execution.view_in<givm::execution_state::dice_selection>().select(library, table, random, {});
    execution.view_in<givm::execution_state::dice_selection>().select(library, table, random, {});
    std::println("回合开始通知后仍受控: {}", library.is_controlled(table[target]));
}
```

输出

```text
投骰时出战角色受控: true
冻结属于控制: true
回合开始通知后仍受控: false
```
