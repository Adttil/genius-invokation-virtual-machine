[givm](../../reference.md) / [执行](../executor.md) / **execution_state**

# givm::execution_state

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class execution_state : std::uint8_t
{
    finished = 1,
    card_selection,
    initial_card_selection,
    initial_active_character_selection,
    remaining_active_character_selection,
    dice_selection,
    action_selection,
    health_reduced,
    active_character_changed,
    initial_active_characters_selected,
    round_started,
    action_started,
    round_end_declared,
    round_ending
};
```

执行器交回控制权时的对局现场种类，涵盖需要输入的选择、可观察的领域变化和终局。

## 枚举值

| | |
| --- | --- |
| `finished` | 已结束的对局 |
| `card_selection` | 等待指定玩家换牌的现场 |
| `initial_card_selection` | 开局尚未接受任何一方换牌选择的现场 |
| `initial_active_character_selection` | 开局尚未接受任何一方出战角色选择的现场 |
| `remaining_active_character_selection` | 开局已经接受一方选择、等待另一方选择出战角色的现场 |
| `dice_selection` | 等待骰子重投选择的现场 |
| `action_selection` | 等待选择行动的现场 |
| `health_reduced` | 非零伤害扣除生命后的现场 |
| `active_character_changed` | 一名玩家即将设置出战角色的现场 |
| `initial_active_characters_selected` | 开局双方出战角色已经同时设置完成的现场 |
| `round_started` | 回合数已经更新、尚未检查回合上限及清空骰子的现场 |
| `action_started` | 当前玩家开始一次行动机会的现场 |
| `round_end_declared` | 当前玩家已经宣布本回合结束、尚未处理相应响应的现场 |
| `round_ending` | 即将处理回合结束的现场 |

## 注意

以 [`compile_mode::normal`](compile_mode.md) 编译时，[`step`](executor/step.md) 返回输入现场或 `finished`；以 `compile_mode::observed` 编译时还会返回观察现场。每种现场均可取得相应 [`execution_view`](execution_view.md)。`initial_active_characters_selected` 和四种流程通知的视图不提供额外读取或输入操作，相关信息直接从牌桌读取。

`card_selection` 的玩家已由当前流程确定，不一定是牌桌上的行动方；`initial_card_selection` 和 `initial_active_character_selection` 允许任选先提交的一方。首次出战选择被接受后，`remaining_active_character_selection` 固定等待另一方；双方选择全部被接受后才同时设置出战角色。以观察模式编译时，先报告 `initial_active_characters_selected`，随后推进才处理相应变更响应。

`active_character_changed` 时，目标角色已经确定，牌桌上仍保留原出战角色。通过相应[视图](execution_view/active_character_changed.md)取得目标后，可按其所属玩家直接读取原出战角色；下一次推进才写入目标并处理变更响应。主动切人的支付及资源变化响应在此现场之前完成。

`round_started` 时，`round_number` 已增加；若超过上限，下一次推进才返回 `finished`。未超限时，后续推进先清空双方骰子，再广播同名的 [`round_started`](../definition/events/round_started.md) 规则事件；冻结在该规则事件中解除，观察现场到达时尚未解除。

`action_started` 时，`active_player` 是当前获得行动机会的玩家：首次行动在行动阶段开始的响应结束后报告；战斗行动结束后再次报告，即使另一方已经宣布结束、仍由同一玩家行动；快速行动后不重复报告。以上行动通知均早于该次 [`before_action`](../definition/events/before_action.md) 响应。

`round_end_declared` 时，`active_player` 仍是宣布结束的一方，结束声明标记已经写入，随后推进才处理 [`round_end_declared`](../definition/events/round_end_declared.md) 响应。第一方的结束响应完成、行动机会交给另一方后，再报告 `action_started`。`round_ending` 时，`active_player` 仍是最后宣布结束的一方，结束声明标记尚未清除；随后推进才准备下一回合的先手并处理 [`round_ended`](../definition/events/round_ended.md) 响应。

`finished` 是唯一的终局标记，具体胜负由相应视图的 `result()` 给出。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }, givm::compile_mode::observed);
    givm::table table{};
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    auto state = execution.step(library, table, random);
    while(state == givm::execution_state::round_started)
    {
        // 回合通知直接从牌桌取得信息。
        std::println("进入回合: {}", table.state().round_number);
        state = execution.step(library, table, random);
    }
    std::println("超过回合上限后结束: {}", state == givm::execution_state::finished);
}
```

输出

```text
进入回合: 1
进入回合: 2
超过回合上限后结束: true
```

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](executor/view_in.md) | 取得指定种类的当前现场 |
