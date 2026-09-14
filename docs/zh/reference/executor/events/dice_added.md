[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **dice_added**

# givm::dice_added

定义于头文件 `<givm/executor.hpp>`

```cpp
struct dice_added;
```

元素骰增加后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `const player_id` | 这次事件对应的玩家；只读 |
| `dice` | `const dice_counts` | 本次获得的骰子及数量；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::dice_counts dice{};
    dice[givm::elemental_dice::hydro] = 2;
    givm::dice_added event{ .player = givm::player_id{ 0 }, .dice = dice };
    std::println("本次骰子数量: {}", event.dice.total());
}
```

输出

```text
本次骰子数量: 2
```
