[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **cost_of_switch**

# givm::cost_of_switch

定义于头文件 `<givm/definition.hpp>`

```cpp
struct cost_of_switch;
```

主动切换出战角色的费用计算事件。响应者可以调整所需骰子和行动速度，并记录本次减费内容。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 这次切换的目标角色；只读 |
| `requirement` | [`action_cost_requirement`](action_cost_requirement.md) | 切换的骰子费用和行动速度 |
| `effect_argument` | `cost_effect_argument<cost_of_switch>` | 当前费用响应记录的减费内容 |

## 注意

目标在建立候选时确定，每次重新计算都对应同一角色。费用响应不得使用随机数；调用随机函数属于未定义行为。需要在确认行动后执行的效果由响应返回的 [`handler_program_entry_t`](../handler_program_entry_t.md) 表达，预览费用时不会执行这些效果。

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
| [`begin_action`](../commands/begin_action.md) | 行动阶段的处理命令 |
