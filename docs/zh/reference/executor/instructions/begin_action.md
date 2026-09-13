[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **begin_action**

# givm::begin_action

定义于头文件 `<givm/executor/instructions/begin_action.hpp>`

```cpp
struct begin_action;
```

行动阶段的处理指令，涵盖玩家选择行动至双方宣布结束的过程。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 注意

先发出 [`action_phase_started`](../events/action_phase_started.md)，每次选择行动前发出 [`before_action`](../events/before_action.md)。支持主动切换出战角色和宣布结束；当前行动方必须已有出战角色。主动切换可经过费用计算、支付、出战角色变更及行动权交接。双方均宣布结束后，本指令才结束行动阶段。

等待行动输入时，执行器返回 `execution_state::action`，通过相应的[现场视图](../execution_view/action.md)预览费用、执行行动或宣布结束。主动切换时，候选下标按当前存活非出战角色的遍历顺序选择目标；[`action_argument`](action_argument.md) 包含要支付的骰子，调用方负责保证支付满足计算出的费用。宣布结束无需支付骰子。

只请求计算切换费用时，下一次推进完成计算后再次返回行动现场；通过新取得的视图读取费用，再提交要执行的行动。

以 [`step`](../executor/step.md) 推进主动切人时，在写入新出战角色之前返回 `execution_state::active_character_changed`。相应[视图](../execution_view/active_character_changed.md)给出目标，牌桌仍可读取原出战角色；随后推进先完成设置，再处理变更响应。到达此现场前，已确认的费用响应、骰子支付及 [`dice_removed`](../events/dice_removed.md) 响应均已完成。

以 [`step`](../executor/step.md) 推进时，每次新的行动机会先返回 `execution_state::action_started`，随后才处理 `before_action`。快速行动不结束当前机会；战斗行动结束后，即使另一方已经宣布结束、仍由当前玩家行动，也会报告新的行动机会。宣布结束时先返回 `execution_state::round_end_declared`，此时牌桌上的 `active_player` 仍是宣布结束的一方，随后推进才处理其结束响应。

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
        std::tuple{ givm::begin_action{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    const auto definition = ids.get_id<givm::character_view>("character");
    const auto attacker = table[givm::player_id{ 0 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    const auto target = table[givm::player_id{ 1 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    table[givm::player_id{ 0 }].state().active_character = attacker;
    table[givm::player_id{ 1 }].state().active_character = target;
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    auto state = execution.run(table, random);
    int declarations = 0;
    while(state == givm::execution_state::action)
    {
        // 当前玩家宣布本回合结束。
        execution.view_in<givm::execution_state::action>().declare_round_end();
        ++declarations;
        state = execution.run(table, random);
    }
    std::println("双方结束声明次数: {}", declarations);
}
```

输出

```text
双方结束声明次数: 2
```

## 参阅

| | |
| --- | --- |
| [`action_phase_started`](../events/action_phase_started.md) | 本回合行动阶段开始的通知 |
| [`before_action`](../events/before_action.md) | 当前行动玩家选择行动前的事件 |
| [`cost_of_switch`](../events/cost_of_switch.md) | 主动切换出战角色的费用计算事件 |
| [`round_end_declared`](../events/round_end_declared.md) | 玩家宣布本回合结束的通知 |
