[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **start_round**

# givm::start_round

定义于头文件 `<givm/definition.hpp>`

```cpp
struct start_round;
```

新回合的开始命令，负责回合数更新、双方元素骰的清空和回合开始效果。超过回合上限时，结束对局并判定双方失败。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `max_rounds` | `std::uint32_t` | 允许进行的最大回合数，初始为 14 |

## 注意

先增加回合数，再判断是否超过上限；超过时以双方失败结束，不清空骰子，也不广播回合开始事件。因此因回合数超限而结束时，牌桌回合数为 `max_rounds + 1`。

未超过上限时，先清空双方骰子，再广播 [`round_started`](../events/round_started.md)，完整处理其响应及返回程序。冻结在此规则通知中解除；掷骰子由独立的 [`start_dice_roll_phase`](start_dice_roll_phase.md) 安排。

以 [`compile_mode::observed`](../../executor/compile_mode.md) 编译时，增加回合数后先返回 `execution_state::round_started`，随后推进才判断上限、清空骰子和处理同名规则事件。因此观察到回合开始时，冻结尚未因该规则事件解除。

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
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 2 } }, givm::compile_mode::normal);
    givm::table table{};
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
