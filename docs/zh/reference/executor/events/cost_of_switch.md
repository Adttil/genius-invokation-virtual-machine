[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **cost_of_switch**

# givm::cost_of_switch

定义于头文件 `<givm/executor.hpp>`

```cpp
struct cost_of_switch;
```

主动切换出战角色的费用计算事件。响应者可以调整切换目标、所需骰子和行动速度，并记录本次减费内容。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | [`character_id`](../../table/character_id.md) | 这次切换的目标角色 |
| `requirement` | [`action_cost_requirement`](action_cost_requirement.md) | 切换的骰子费用和行动速度 |
| `effect_argument` | `cost_effect_argument<cost_of_switch>` | 当前费用响应记录的减费内容 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::cost_of_switch event{ .target = { .player_id = givm::player_id{ 0 }, .index = 1 } };
    event.requirement.dice_requirement.any = 1;
    // 一次效果把切换改为无需骰子的快速行动。
    --event.requirement.dice_requirement.any;
    event.requirement.speed = givm::action_speed::fast;
    event.effect_argument.reduced_dice.any = 1;
    std::println("所需骰数: {}", event.requirement.dice_requirement.any);
    std::println("快速行动: {}", event.requirement.speed == givm::action_speed::fast);
}
```

输出

```text
所需骰数: 0
快速行动: true
```

## 参阅

| | |
| --- | --- |
| [`begin_action`](../instructions/begin_action.md) | 行动阶段的处理指令 |
