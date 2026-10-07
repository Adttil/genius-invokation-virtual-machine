[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **cost_of_switch**

# givm::cost_of_switch

定义于头文件 `<givm/definition.hpp>`

```cpp
struct cost_of_switch;
```

主动切换出战角色的费用计算事件。响应者可以调整所需骰子和行动速度。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 这次切换的目标角色；只读 |
| `requirement` | [`action_cost_requirement`](../../table/action_cost_requirement.md) | 切换的骰子、充能费用和行动速度 |

## 注意

同一行动窗口内，每个切换目标只能计算一次费用，之后通过返回的报价标识反复读取；Debug 检查重复计算，Release 由调用方保证。所有费用响应读取报价期间不变的牌桌，前一响应只通过费用事件影响后一响应；已提交效果不会在报价时修改牌桌。

目标在建立候选时确定，计算期间不能修改目标角色。预览上下文没有 `random()`；后续效果通过 `return context.invoke(entry, inputs...);` 缓存，没有手动选择保存位置的标记。预览时不执行效果，确认行动后才执行。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::cost_of_switch event{ .target = { givm::player_id{ 0 }, 1 } };
    event.requirement.dice_requirement.any = 1;
    // 一次效果把切换改为无需骰子的快速行动。
    --event.requirement.dice_requirement.any;
    event.requirement.speed = givm::action_speed::fast;
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
