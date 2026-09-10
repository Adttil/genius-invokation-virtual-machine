[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **dice_removed**

# givm::dice_removed

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct dice_removed;
```

元素骰移除或支付后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `const player_id` | 这次事件对应的玩家；只读 |
| `dice` | `const dice_counts` | 本次失去或支付的骰子及数量；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::dice_counts dice{};
    dice[givm::elemental_dice::hydro] = 2;
    givm::dice_removed event{ .player = givm::player_id{ 0 }, .dice = dice };
    std::println("本次骰子数量: {}", event.dice.total());
}
```

输出

```text
本次骰子数量: 2
```
