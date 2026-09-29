[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **end_round**

# givm::end_round

定义于头文件 `<givm/definition.hpp>`

```cpp
struct end_round;
```

回合结束命令，负责本回合的收尾与下一回合的先手准备。它应放在双方的结束声明已经结算完毕之后。

## 成员类型

| | |
| --- | --- |
| [`error_type`](#编译检查) | `end_round_error` 的别名，即本命令的编译检查错误类型 |

## 编译检查

```cpp
enum class end_round_error {};
```

`end_round::error_type` 是 `givm::end_round_error` 的别名。这是没有枚举项的空枚举类型，本命令没有编译期参数错误。

## 注意

将行动玩家从最后宣布结束的一方切换为另一方，清除已有人宣布结束的标记，然后发出 [`round_ended`](../events/round_ended.md)。回合结束抽牌等其他效果可在本命令之后另行安排。

以 [`compile_mode::observed`](../../executor/compile_mode.md) 编译时，执行上述操作前先返回 `execution_state::round_ending`。此时 `active_player` 仍是最后宣布结束的一方，结束声明标记尚未清除。

## 示例

```cpp
#include <utility>
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
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{ givm::select_active_character_both{}, givm::begin_action{}, givm::end_round{} },
        std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 0 } };
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition } },
        givm::linked_deck{ .characters = { definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 0 }, 0 });
    auto state = execution.view_in<givm::execution_state::remaining_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 1 }, 0 });
    while(state == givm::execution_state::action_selection)
    {
        // 当前玩家宣布本回合结束。
        state = execution.view_in<givm::execution_state::action_selection>().declare_round_end(library, table, random);
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
