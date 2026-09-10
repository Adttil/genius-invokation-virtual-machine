[givm](../../reference.md) / [游戏用语](../enums.md) / **action_speed**

# givm::action_speed

定义于头文件 `<givm/enums/action_speed.hpp>`

```cpp
enum class action_speed : std::uint8_t
{
    fast,
    combat
};
```

行动的速度类别，用于决定行动完成后由谁继续行动。

## 枚举值

|  |  |
| --- | --- |
| `fast` | 快速行动，完成后由当前玩家继续行动 |
| `combat` | 战斗行动；对方尚未宣布回合结束时，完成后将行动权交给对方 |

## 示例

```cpp
#include <print>

#include <givm/executor/events.hpp>

int main()
{
    givm::action_cost_requirement requirement{};
    std::println("原费用对应战斗行动: {}", requirement.speed == givm::action_speed::combat);
    // 将本次行动的费用要求调整为快速行动。
    requirement.speed = givm::action_speed::fast;
    std::println("调整后为快速行动: {}", requirement.speed == givm::action_speed::fast);
}
```

输出

```text
原费用对应战斗行动: true
调整后为快速行动: true
```
