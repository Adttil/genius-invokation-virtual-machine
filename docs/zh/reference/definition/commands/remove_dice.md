[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_dice**

# givm::remove_dice

定义于头文件 `<givm/definition.hpp>`

```cpp
struct remove_dice
{
    using input_type = remove_dice_input;

    relative_player player = static_cast<relative_player>(-1);
    dice_counts dice{};
};
```

从一位玩家的骰池中扣除指定种类和数量的元素骰的命令。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`remove_dice_input`](../command_inputs/remove_dice_input.md)，动态模式下的输入类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`relative_player`](relative_player.md) | 固定模式下失去元素骰的一方；默认采用动态输入 |
| `dice` | [`dice_counts`](../../enums/dice_counts.md) | 固定模式下本次扣除的各类元素骰数量 |

## 注意

默认构造 `remove_dice{}` 使用动态模式，由响应通过 [`invoke`](../../executor/handle_context/invoke.md) 提交一个 [`remove_dice_input`](../command_inputs/remove_dice_input.md)，包含玩家 ID 和本次扣除的各类骰子数量。显式指定 `player` 为 `self` 或 `opponent` 时采用固定模式，不消费响应输入。本方的含义见 [`relative_player`](relative_player.md)；动态输入的玩家 ID 必须有效。

调用方必须保证命令执行时玩家每类骰子的数量都足够扣除。命令精确扣除指定数量，不自动减少扣除量；需要“至多扣除”的效果时，由响应根据骰池计算实际数量后提交动态输入。

一次性扣除全部指定骰子，然后全场广播一次 [`dice_removed`](../events/dice_removed.md)。事件记录本次扣除量；响应读取牌桌时可以看到扣除后的骰池。全部数量为零时无效果，也不广播。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::dice_counts initial_dice{};
    initial_dice[givm::elemental_dice::pyro] = 3;
    initial_dice[givm::elemental_dice::omni] = 2;
    givm::dice_counts collected_dice{};
    collected_dice[givm::elemental_dice::pyro] = 1;
    collected_dice[givm::elemental_dice::omni] = 1;
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    const auto [library, ids] = compile(sources,
        std::tuple{
            givm::add_dice{ .player = givm::relative_player::self, .dice = initial_dice },
            givm::remove_dice{ .player = givm::relative_player::self, .dice = collected_dice },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);

    givm::table table{ { .self_player = givm::player_id{ 1 } } };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.start(library, table);
    execution.step(library, table, random);
    const auto& remaining = table[givm::player_id{ 1 }].state().dice;
    std::println("剩余火元素骰: {}", remaining[givm::elemental_dice::pyro]);
    std::println("剩余万能元素骰: {}", remaining[givm::elemental_dice::omni]);
}
```

输出

```text
剩余火元素骰: 2
剩余万能元素骰: 1
```

## 参阅

| | |
| --- | --- |
| [`dice_removed`](../events/dice_removed.md) | 元素骰移除或支付后的通知 |
| [`add_dice`](add_dice.md) | 增加指定元素骰的命令 |
