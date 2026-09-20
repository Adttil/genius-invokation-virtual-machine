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

同一行动窗口内，每个候选只能计算一次费用，之后可反复读取结果；库不检查重复计算。所有费用响应读取报价期间不变的牌桌，前一响应只通过费用事件影响后一响应；已提交效果不会在报价时修改牌桌。

目标在建立候选时确定，计算期间不能修改目标角色。费用响应不得使用随机数；调用随机函数属于未定义行为。需要确认行动后执行的效果由响应通过 [`handle_context::invoke`](../../executor/handle_context/invoke.md) 提交，必须采用 `return context.invoke(givm::substack_t{}, entry, inputs...);` 的形式，没有输入时也须传这个标记。预览费用时仅保留入口和输入，不执行这些效果；误用普通重载属于未定义行为，不进行运行期检查。

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
