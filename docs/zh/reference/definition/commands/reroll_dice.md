[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **reroll_dice**

# givm::reroll_dice

定义于头文件 `<givm/definition.hpp>`

```cpp
struct reroll_dice
{
    using input_type = reroll_dice_input;

    relative_player player = static_cast<relative_player>(-1);
    std::uint32_t reroll_count = 1;
};
```

指定玩家对当前持有的元素骰进行至多若干次重投的命令。每次由玩家独立选择各类骰子的重投数量。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`reroll_dice_input`](../command_inputs/reroll_dice_input.md)，动态模式下的输入类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`relative_player`](relative_player.md) | 固定模式下进行重投的一方；默认采用动态输入 |
| `reroll_count` | `std::uint32_t` | 固定模式下最多可重投的次数，初始为 1 |

## 注意

默认构造 `reroll_dice{}` 使用动态模式，由响应通过 [`invoke`](../../executor/handle_context/invoke.md) 提交一个 [`reroll_dice_input`](../command_inputs/reroll_dice_input.md)，包含玩家 ID 和重投次数。显式指定 `player` 为 `self` 或 `opponent` 时采用固定模式，不消费响应输入。本方的含义见 [`relative_player`](relative_player.md)；动态输入的玩家 ID 必须有效。

若次数为零或该玩家当前没有骰子，本命令立即完成，不要求选择，也不调用随机源。否则返回 [`execution_state::dice_reroll_selection`](../../executor/execution_state.md)，通过相应[现场视图](../../executor/execution_view/dice_reroll_selection.md)提交选择。该玩家的每类所选数量不得超过当前持有数量，可以通过 `selection_validate` 独立检查；提交和后续推进不自动检查。

每次非空选择只替换所选骰子，并消耗一次重投机会。只要还有机会，就再次等待该玩家选择；各次可以选择不同骰子。空选择放弃全部剩余机会。重投不改变骰子总数，不广播准备、增加、移除、转换或其他事件。

### 随机结果

以命令开始时的骰子总数为 `C`、重投次数为 `R`。第一次返回选择现场前，一次性从随机源取得 `ceil(C × R / 10)` 个 `std::uint32_t` 值，其中 `ceil` 表示向上取整。每个值 `r` 按从低到高的顺序提供十个结果：第 `i` 个结果为 `floor(r / 2^(3 × i)) % 8`，`i` 从 `0` 到 `9`，最高两位不使用。结果 `0` 至 `7` 依次对应冰、水、火、雷、岩、草、风、万能元素骰，与 [`elemental_dice_from_random`](../../enums/elemental_dice_from_random.md) 的映射相同。

每次重投 `k` 个骰子，连续使用尚未使用的前 `k` 个结果。再次重投接着使用，不跳过未选骰子对应的数量，也不重新从下一个随机值开始。后续提交选择不再调用随机源；放弃后未使用的结果弃用。复制牌桌和执行器保留这些已取得的结果，因此更换后续推进的随机源不会改变它们。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    const auto [library, ids] = compile(sources, basics,
        std::tuple{
            givm::reroll_dice{ .player = givm::relative_player::self, .reroll_count = 2 },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);

    givm::dice_counts initial{};
    initial[givm::elemental_dice::cryo] = 2;
    initial[givm::elemental_dice::omni] = 1;
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .dice = initial } };
    givm::executor execution{};
    std::uint32_t random_calls = 0;
    auto random = [&]() -> std::uint32_t { ++random_calls; return 17; };
    execution.start(library, table);
    auto state = execution.step(library, table, random);
    while(state == givm::execution_state::dice_reroll_selection)
    {
        const auto view = execution.view_in<givm::execution_state::dice_reroll_selection>();
        givm::dice_counts selected{};
        selected[givm::elemental_dice::cryo] = 1;
        if(not view.selection_validate(table, selected))
            return 1;
        view.select(selected);
        state = execution.step(library, table, random);
    }
    const auto& dice = table[givm::player_id{ 0 }].state().dice;
    std::println("水、火、万能: {}、{}、{}", dice[givm::elemental_dice::hydro],
        dice[givm::elemental_dice::pyro], dice[givm::elemental_dice::omni]);
    std::println("随机调用次数: {}", random_calls);
}
```

输出

```text
水、火、万能: 1、1、1
随机调用次数: 1
```

## 参阅

| | |
| --- | --- |
| [`start_dice_roll_phase`](start_dice_roll_phase.md) | 双方投骰阶段的处理命令 |
| [`execution_view<dice_reroll_selection>`](../../executor/execution_view/dice_reroll_selection.md) | 单方重投选择现场的视图 |
