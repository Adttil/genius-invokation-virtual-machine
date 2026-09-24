[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view](../../execution_view.md) / [health_reduced](../health_reduced.md) / **replacement_reaction**

# givm::execution_view<execution_state::health_reduced>::replacement_reaction

定义于头文件 `<givm/executor.hpp>`

```cpp
tag_id replacement_reaction() const noexcept;
```

取得本次元素反应采用的替代标签，用于识别反应效果由哪一种规则替代。原始反应种类仍由 [`reaction()`](reaction.md) 返回。

## 返回值

已确定的只读 [`tag_id`](../../../table/tag_id.md)；空标签表示使用默认反应效果。没有反应时也为空，需结合 `reaction()` 区分。

## 注意

标签在伤害属性确定、反应判定完成后，通过 [`elemental_reaction_will_occur`](../../../definition/events/elemental_reaction_will_occur.md) 选择，并在数值计算开始前固定。本现场只能读取标签，不能重新选择。

非空标签取消默认反应加伤、派生伤害、实体生成与超载切人，但不改变原始反应，也不影响默认附着处理。到达本现场时，本段伤害已经采用该标签完成数值计算，整组元素附着也已在准备阶段推进；本段扣血后的默认反应效果尚未处理。

## 示例

下面观察一次默认融化伤害。反应种类为融化，替代标签为空。

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
        return { .max_health = 10, .max_energy = 3, .health = 10 };
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
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    const std::array damages{
        givm::fixed_damage{
            .source = givm::relative_character_target{ givm::relative_player::self, 0 },
            .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::pyro }
    };
    const auto [library, ids] = compile(sources,
        std::tuple{
            givm::select_active_character_both{},
            givm::apply_element{
                .source = givm::relative_character_target{ givm::relative_player::self, 0 },
                .target = givm::relative_character_target{ givm::relative_player::opponent, 0 },
                .element = givm::element::cryo },
            givm::deal_damage{ .damages = damages }
        },
        std::tuple{}, givm::compile_mode::observed);
    givm::table table{ { .max_rounds = 0, .self_player = givm::player_id{ 0 } } };
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition } },
        givm::linked_deck{ .characters = { definition } });
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
    if(execution.step(library, table, random) == givm::execution_state::health_reduced)
    {
        const auto view = execution.view_in<givm::execution_state::health_reduced>();
        std::println("原始反应是融化: {}", view.reaction() == givm::elemental_reaction::melt);
        std::println("使用默认反应效果: {}", not view.replacement_reaction());
    }
}
```

输出

```text
原始反应是融化: true
使用默认反应效果: true
```

## 参阅

| | |
| --- | --- |
| [`reaction`](reaction.md) | 取得原始元素反应种类 |
| [`issued_id_map::get_tag_id`](../../../definition/issued_id_map/get_tag_id.md) | 按名称取得可用于比较的标签 ID |
