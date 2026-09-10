[givm](../../reference.md) / [事件](../events.md) / **dice_roll_preparation**

# givm::dice_roll_preparation

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct dice_roll_preparation;
```

本回合开始投骰前的准备事件。响应者可以为双方指定必定获得的骰子，并调整能够重投的次数。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `count` | `const std::uint32_t` | 每名玩家本次获得的骰子总数；只读 |
| `fixed_dice` | `std::array<dice_counts, 2>` | 双方必定获得的骰子，按玩家编号索引；每方总数不得超过 count |
| `reroll_count` | `std::array<std::uint32_t, 2>` | 双方能够重投的次数，按玩家编号索引；初始各为 1 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::dice_roll_preparation event{ .count = 8 };
    event.fixed_dice[0][givm::elemental_dice::omni] = 2;
    ++event.reroll_count[0];
    std::println("固定万能骰: {}", event.fixed_dice[0][givm::elemental_dice::omni]);
    std::println("剩余随机骰数: {}", event.count - event.fixed_dice[0].total());
    std::println("重投次数: {}", event.reroll_count[0]);
}
```

输出

```text
固定万能骰: 2
剩余随机骰数: 6
重投次数: 2
```

## 参阅

| | |
| --- | --- |
| [`start_dice_roll_phase`](../instructions/start_dice_roll_phase.md) | 开始投骰 |
