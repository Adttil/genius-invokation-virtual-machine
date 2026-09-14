[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **select_active_character_both**

# givm::select_active_character_both

定义于头文件 `<givm/executor.hpp>`

```cpp
struct select_active_character_both;
```

双方开局出战角色的选择指令。双方提交后，同时设置两边的出战角色，再通知相关效果。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 注意

尚未接受任何一方的选择时，执行器返回 `execution_state::initial_active_character_selection`，通过相应的[现场视图](../execution_view/initial_active_character_selection.md)提交任意一方的有效角色。首次选择被接受后返回 `execution_state::remaining_active_character_selection`；此时相应[视图](../execution_view/remaining_active_character_selection.md)提供已接受的选择和待选玩家，第二次输入只指定该玩家的有效角色下标。

第一次选择被接受后仍未设置出战角色，第二次选择被接受后才同时生效。随后先发出玩家 0 的 [`active_character_changed`](../events/active_character_changed.md)，其响应及后续效果完成后再发出玩家 1 的通知；顺序不受双方提交选择的先后影响。响应若结束对局，后续通知不再进行。

以 [`step`](../executor/step.md) 推进时，双方结果同时生效后先返回 `execution_state::initial_active_characters_selected`。该通知的视图不提供额外读取或输入操作，双方结果直接从牌桌读取；随后推进才处理变更响应。

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

    static givm::program_entry<givm::character_initialization> handle(
        const definition_type&, const givm::character_view&,
        givm::character_initialization& event, const givm::card_table&, givm::random_fn&)
    {
        event.state = { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
        return givm::program_entry<givm::character_initialization>::null();
    }
};

int main()
{
    character_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::initialize_characters{ .player = givm::player_id{ 0 } }, givm::initialize_characters{ .player = givm::player_id{ 1 } }, givm::select_active_character_both{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{};
    const auto definition = ids.get_id<givm::character_view>("character");
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .characters = { definition } });
    table.load_deck(givm::player_id{ 1 }, givm::linked_deck{ .characters = { definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(target);
    execution.step(library, table, random);
    const auto remaining = execution.view_in<givm::execution_state::remaining_active_character_selection>();
    std::println("首份选择已接受: {}", remaining.selected() == target);
    std::println("剩余玩家为玩家 0: {}", remaining.player() == givm::player_id{ 0 });
    remaining.select(attacker.index);
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
