[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **start_dice_roll_phase**

# givm::start_dice_roll_phase

定义于头文件 `<givm/definition.hpp>`

```cpp
struct start_dice_roll_phase;
```

双方投骰阶段的处理命令，涵盖固定骰子与重投次数的准备、初次投骰和重投选择。

## 成员类型

| | |
| --- | --- |
| `input_type` | `void`，表示不消费调用输入 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `count` | `std::uint32_t` | 每方投出的骰子总数，初始为 8 |
| `reroll_count` | `std::array<std::uint32_t, 2>` | 双方重投次数，初始各为 1 |

## 注意

先发出 [`dice_roll_preparation`](../events/dice_roll_preparation.md)，完成其响应及后续效果后，再按最终事件值进行投骰。以下以 `C` 表示每方骰子总数，`F0`、`F1` 表示双方固定骰子总数，`R0`、`R1` 表示双方重投次数。每方固定骰子总数不得超过 `C`，`C` 不得超过 64。

每个随机值 `r` 来自本次推进传入的随机源，经 [`random_fn`](../../executor/random_fn.md) 取得，取值范围为 `0` 至 `2^32 - 1`。下文的随机调用数量只包括投骰与重投自身；准备事件的响应及其后续效果所作的随机调用另计，并先于本次投骰发生。若这些响应结束对局，则不再投骰。

### 首次投骰

先处理玩家 0，再处理玩家 1。每方先取得准备事件指定的固定骰子，其余每个骰子调用随机源一次，按 [`elemental_dice_from_random`](../../enums/elemental_dice_from_random.md) 映射：

| `r % 8` | 骰子 |
| --- | --- |
| `0` | 万能 |
| `1` | 冰 |
| `2` | 水 |
| `3` | 火 |
| `4` | 雷 |
| `5` | 风 |
| `6` | 岩 |
| `7` | 草 |

因此，首次投骰依次使用玩家 0 的 `C - F0` 个值和玩家 1 的 `C - F1` 个值。固定骰子本身不消耗随机值；“固定”只约定首次投骰结果，仍可以被选中重投。

### 重投结果的预先分配

双方首次投骰完成后，在首次返回 `execution_state::dice_selection` 前，先为玩家 0 取得 `ceil(C × R0 / 10)` 个值，再为玩家 1 取得 `ceil(C × R1 / 10)` 个值。这里 `ceil` 表示向上取整；乘积为零时不取值。即使玩家随后放弃重投，这些调用也已经发生。

每个值提供按顺序排列的十个骰子结果。第 `i` 个结果为 `floor(r / 2^(3 × i)) % 8`，其中 `i` 从 `0` 到 `9`，`floor` 表示向下取整；骰子种类与上表相同。因此低三位先使用，随后每三位一组，最高两位不用于骰子结果。例如 `r = 17` 的前三个结果依次为冰、水、万能。

每方独立地连续使用自己获配的结果序列。一次选择重投 `k` 个骰子，就依次取该方尚未使用的前 `k` 个结果；再次重投接着使用，不跳过未选择的骰子数，也不从下一个随机值重新开始。一个随机值提供的十个结果全部使用后，才继续到该方下一个值。玩家 0 未使用的结果不会交给玩家 1。

复制牌桌和执行器会保留已经分配的随机结果；在重投输入现场更换后续推进使用的随机源，不会改变这些结果。记录随机序列时，须包含所有已调用取得的值，即使最终没有用完。

### 重投选择

等待重投时，执行器返回 `execution_state::dice_selection`，通过相应的[现场视图](../../executor/execution_view/dice_selection.md)提交选择。选择以 [`dice_counts`](../../enums/dice_counts.md) 指定每种骰子要重投的数量，各类数量不得超过当前持有数量；可先使用视图的 `selection_validate` 独立检查。非空选择移除选中的骰子，再加入上述新结果，并消耗该方一次重投机会。所有数量为零的选择放弃该方全部剩余机会；不再使用的预分配结果弃用。

默认提示仍有重投机会的玩家 0，否则提示玩家 1；调用方可以指定任意仍有机会的玩家先提交，提交顺序不改变各方获配的随机结果序列。双方均无重投机会时，继续后续流程。

提交选择后的重投操作不再调用随机源。整个投骰命令自身的调用总数为：

`(C - F0) + (C - F1) + ceil(C × R0 / 10) + ceil(C × R1 / 10)`。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::start_dice_roll_phase{ .count = 8 } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    // 该随机源使首次投骰全部为万能骰，双方各重投一颗。
    givm::dice_counts selected{};
    selected[givm::elemental_dice::omni] = 1;
    for(int submission = 0; submission < 2; ++submission)
    {
        const auto view = execution.view_in<givm::execution_state::dice_selection>();
        if(not view.selection_validate(table, selected))
            return 1;
        view.select(selected);
        execution.step(library, table, random);
    }
    std::println("玩家 0 的骰子数量: {}", table[givm::player_id{ 0 }].state().dice.total());
    std::println("玩家 1 的骰子数量: {}", table[givm::player_id{ 1 }].state().dice.total());
}
```

输出

```text
玩家 0 的骰子数量: 8
玩家 1 的骰子数量: 8
```

## 参阅

| | |
| --- | --- |
| [`dice_roll_preparation`](../events/dice_roll_preparation.md) | 本回合投骰前的准备事件 |
