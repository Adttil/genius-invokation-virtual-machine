[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **start_round**

# givm::start_round

定义于头文件 `<givm/definition.hpp>`

```cpp
struct start_round;
```

发送新回合开始的规则通知，供冻结解除、支援返还资源等回合开始效果响应。

## 结算

本命令广播 [`round_started`](../events/round_started.md)，各响应及其返回程序完整结算后继续下一条命令。命令没有参数；不更新回合数、不清空骰子，也不自行处理投骰。

[根回合流程](../../executor/compile.md) 每次开始时自动增加回合数、检查 [`game_parameters::max_rounds`](../../table/game_parameters.md) 并清空旧骰子。调用方在回合命令序列中依次安排 [`start_dice_roll_phase`](start_dice_roll_phase.md)、`start_round{}`，即可让规则通知在双方投骰及全部重投后发生。冻结到本通知时才解除，投骰阶段仍保留。

观察模式的 `execution_state::round_started` 由根回合推进产生，位于回合数增加之后、上限检查之前。本命令只发送规则通知，不额外返回同名观察现场。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
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
        std::tuple{},
        std::tuple{
            givm::start_dice_roll_phase{ .count = 0, .reroll_count = { 0, 0 } },
            givm::start_round{}
        }, givm::compile_mode::normal);
    givm::table table{ { .max_rounds = 2 } };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    std::println("回合数: {}", table.state().round_number);
    std::println("超过上限后双败: {}", execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
}
```

输出

```text
回合数: 3
超过上限后双败: true
```
