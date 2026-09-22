[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **select_active_character_both**

# givm::select_active_character_both

定义于头文件 `<givm/definition.hpp>`

```cpp
struct select_active_character_both;
```

双方开局出战角色的选择命令。双方提交后，同时设置两边的出战角色，再通知相关效果。

## 注意

尚未接受任何一方的选择时，执行器返回 `execution_state::initial_active_character_selection`，通过相应的[现场视图](../../executor/execution_view/initial_active_character_selection.md)提交任意一方的有效角色。首次选择被接受后返回 `execution_state::remaining_active_character_selection`；此时相应[视图](../../executor/execution_view/remaining_active_character_selection.md)提供已接受的选择和待选玩家，第二次输入提交该玩家的有效角色 ID。两种视图都提供独立的 `selection_validate`，提交及继续推进不会自动检查。

第一次选择被接受后仍未设置出战角色，第二次选择被接受后才同时生效。随后先发出玩家 0 的 [`active_character_changed`](../events/active_character_changed.md)，其响应及后续效果完成后再发出玩家 1 的通知；顺序不受双方提交选择的先后影响。响应若结束对局，后续通知不再进行。

以 [`compile_mode::observed`](../../executor/compile_mode.md) 编译时，双方结果同时生效后先返回 `execution_state::initial_active_characters_selected`。该通知的视图不提供额外读取或输入操作，双方结果直接从牌桌读取；随后推进才处理变更响应。

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
    character_source source{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::select_active_character_both{} },
        std::tuple{}, givm::compile_mode::observed);
    givm::table table{ { .max_rounds = 0 } };
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition } },
        givm::linked_deck{ .characters = { definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(target);
    execution.step(library, table, random);
    const auto remaining = execution.view_in<givm::execution_state::remaining_active_character_selection>();
    std::println("首份选择已接受: {}", remaining.first_selected_character() == target);
    std::println("剩余玩家为玩家 0: {}", remaining.player() == givm::player_id{ 0 });
    if(remaining.selection_validate(table, attacker) != givm::remaining_active_character_selection_validation::valid)
        return 1;
    remaining.select(attacker);
    const auto state = execution.step(library, table, random);
    std::println("双方出战角色同时设置完成: {}", state == givm::execution_state::initial_active_characters_selected);
    std::println("玩家 0 已选出战角色: {}", table[givm::player_id{ 0 }].state().active_character == attacker);
    std::println("玩家 1 已选出战角色: {}", table[givm::player_id{ 1 }].state().active_character == target);
}
```

输出

```text
首份选择已接受: true
剩余玩家为玩家 0: true
双方出战角色同时设置完成: true
玩家 0 已选出战角色: true
玩家 1 已选出战角色: true
```

## 参阅

| | |
| --- | --- |
| [`active_character_changed`](../events/active_character_changed.md) | 出战角色设置完成后的通知 |
