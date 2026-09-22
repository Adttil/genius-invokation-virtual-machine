[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **is_controlled**

# givm::definition_library::is_controlled

定义于头文件 `<givm/executor.hpp>`

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
    character_source character{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(character);
    const auto issued = sources.make_issued_id_map();
    const auto frozen = issued.get_id<givm::attachment_view>(givm::genshin_impact::frozen_3_3_0.name());
    const givm::character_id target{ givm::player_id{ 0 }, 0 };
    const auto [library, ids] = compile(sources,
        std::tuple{
            givm::set_active_character{ .target = target },
            givm::set_active_character{ .target = { givm::player_id{ 1 }, 0 } },
            givm::start_round{ .max_rounds = 2 },
            givm::attach{ .definition = frozen },
            givm::begin_action{}, givm::end_round{},
            givm::start_round{ .max_rounds = 2 },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);
    givm::table table{};
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition } },
        givm::linked_deck{ .characters = { definition } });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    std::println("当前出战角色受控: {}", action.is_controlled(library, table));
    std::println("冻结属于控制: {}", library.is_control(library.frozen_id()));
    action.declare_round_end();
    execution.step(library, table, random);
    execution.view_in<givm::execution_state::action_selection>().declare_round_end();
    execution.step(library, table, random);
    std::println("下一回合开始后仍受控: {}", library.is_controlled(table[target]));
}
```

输出

```text
当前出战角色受控: true
冻结属于控制: true
下一回合开始后仍受控: false
```
