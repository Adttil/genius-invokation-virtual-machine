[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **end_round**

# givm::end_round

定义于头文件 `<givm/executor.hpp>`

```cpp
struct end_round;
```

回合结束指令，负责本回合的收尾与下一回合的先手准备。它应放在双方的结束声明已经结算完毕之后。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 注意

将行动玩家从最后宣布结束的一方切换为另一方，清除已有人宣布结束的标记，然后发出 [`round_ended`](../events/round_ended.md)。回合结束抽牌等其他效果可在本指令之后另行安排。

以 [`step`](../executor/step.md) 推进时，执行上述操作前先返回 `execution_state::round_ending`。此时 `active_player` 仍是最后宣布结束的一方，结束声明标记尚未清除。

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
        givm::character_initialization& event, const givm::table&, givm::random_fn&)
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
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::initialize_characters{ .player = givm::player_id{ 0 } }, givm::initialize_characters{ .player = givm::player_id{ 1 } }, givm::set_active_character{ .target = givm::character_id{ givm::player_id{ 0 }, 0 } }, givm::set_active_character{ .target = givm::character_id{ givm::player_id{ 1 }, 0 } }, givm::begin_action{}, givm::end_round{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::table table{};
    const auto definition = ids.get_id<givm::character_view>("character");
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .characters = { definition } });
    table.load_deck(givm::player_id{ 1 }, givm::linked_deck{ .characters = { definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    auto state = execution.run(library, table, random);
    while(state == givm::execution_state::action)
    {
        // 当前玩家宣布本回合结束。
        execution.view_in<givm::execution_state::action>().declare_round_end();
        state = execution.run(library, table, random);
    }
    std::println("下一回合由玩家 0 先手: {}", table.state().active_player == givm::player_id{ 0 });
    std::println("结束声明标记已清除: {}", !table.state().first_ended);
}
```

输出

```text
下一回合由玩家 0 先手: true
结束声明标记已清除: true
```

## 参阅

| | |
| --- | --- |
| [`round_ended`](../events/round_ended.md) | 本回合结束的通知 |
