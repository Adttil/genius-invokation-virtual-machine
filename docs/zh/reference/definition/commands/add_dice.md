[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **add_dice**

# givm::add_dice

定义于头文件 `<givm/definition.hpp>`

```cpp
struct add_dice
{
    relative_player player = static_cast<relative_player>(-1);
    dice_counts dice{};
};
```

向一位玩家增加指定种类和数量的元素骰的命令。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`relative_player`](relative_player.md) | 固定模式下获得元素骰的一方；默认采用动态输入 |
| `dice` | [`dice_counts`](../../enums/dice_counts.md) | 固定模式下本次增加的各类元素骰数量 |

## 注意

默认构造 `add_dice{}` 使用动态模式，由响应通过 `invoke` 提交一个 [`dice_added`](../events/dice_added.md)，包含玩家 ID 和本次增加的各类骰子数量。显式指定 `player` 为 `self` 或 `opponent` 时采用固定模式，不消费响应输入。本方的含义见 [`relative_player`](relative_player.md)；动态输入的玩家 ID 必须有效。

一次性增加全部指定骰子，然后全场广播一次 `dice_added`。事件记录本次增加量；响应读取牌桌时可以看到增加后的骰子。全部数量为零时无效果，也不广播。

命令本身不调用随机源；若需要随机种类或数量，由响应计算后提交动态输入。投骰和元素调和不属于此命令的产骰，也不广播 `dice_added`。

调用方应确保每类骰子增加后的数量在 0 到 255 范围内，与 [`dice_counts::operator+=`](../../enums/dice_counts/operator_add_assign.md) 的要求一致。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::dice_counts self_dice{};
    self_dice[givm::elemental_dice::pyro] = 2;
    self_dice[givm::elemental_dice::omni] = 1;
    givm::dice_counts opponent_dice{};
    opponent_dice[givm::elemental_dice::hydro] = 1;
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    const auto [library, ids] = compile(sources,
        std::tuple{
            givm::add_dice{ .player = givm::relative_player::self, .dice = self_dice },
            givm::add_dice{ .player = givm::relative_player::opponent, .dice = opponent_dice },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);

    givm::table table{ { .self_player = givm::player_id{ 1 } } };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    execution.step(library, table, random);
    const auto& self = table[givm::player_id{ 1 }].state().dice;
    const auto& opponent = table[givm::player_id{ 0 }].state().dice;
    std::println("我方火元素骰: {}", self[givm::elemental_dice::pyro]);
    std::println("我方万能元素骰: {}", self[givm::elemental_dice::omni]);
    std::println("对方水元素骰: {}", opponent[givm::elemental_dice::hydro]);
}
```

输出

```text
我方火元素骰: 2
我方万能元素骰: 1
对方水元素骰: 1
```

## 参阅

| | |
| --- | --- |
| [`dice_added`](../events/dice_added.md) | 产骰的动态输入和增加完成后的通知 |
| [`dice_counts`](../../enums/dice_counts.md) | 各类元素骰的数量 |
