[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **start_dice_roll_phase**

# givm::start_dice_roll_phase

定义于头文件 `<givm/executor/instructions/start_dice_roll_phase.hpp>`

```cpp
struct start_dice_roll_phase;
```

双方投骰阶段的处理指令，涵盖固定骰子与重投次数的准备、初次投骰和重投选择。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `count` | `std::uint32_t` | 每方投出的骰子总数，初始为 8 |
| `reroll_count` | `std::array<std::uint32_t, 2>` | 双方重投次数，初始各为 1 |

## 注意

投骰前发出 [`dice_roll_preparation`](../events/dice_roll_preparation.md)。每方固定骰子总数不得超过 `count`，`count` 不得超过 [`selection_capacity`](../selection_capacity.md)。等待重投时，执行器返回 `execution_state::dice_selection`，通过相应的[现场视图](../execution_view/dice_selection.md)提交选择。选择位按骰子种类顺序展开，同种骰子逐个计数。空选择表示放弃该方剩余重投。每次非空选择消耗一次重投机会。

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
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::start_dice_roll_phase{ .count = 8 } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(table, random);
    // 双方保留首次投出的骰子，不进行重投。
    for(int submission = 0; submission < 2; ++submission)
    {
        execution.view_in<givm::execution_state::dice_selection>().select({});
        execution.run(table, random);
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
