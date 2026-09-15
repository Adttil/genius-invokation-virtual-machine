[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **action_cost_requirement**

# givm::action_cost_requirement

定义于头文件 `<givm/definition.hpp>`

```cpp
struct action_cost_requirement;
```

一项行动的骰子费用与行动速度。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `dice_requirement` | [`elemental_dice_requirement`](elemental_dice_requirement.md) | 这项行动需要的骰子 |
| `speed` | [`action_speed`](../../enums/action_speed.md) | 行动速度 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::action_cost_requirement requirement{};
    requirement.dice_requirement.any = 1;
    requirement.speed = givm::action_speed::fast;
    std::println("任意骰需求: {}", requirement.dice_requirement.any);
    std::println("快速行动: {}", requirement.speed == givm::action_speed::fast);
}
```

输出

```text
任意骰需求: 1
快速行动: true
```
