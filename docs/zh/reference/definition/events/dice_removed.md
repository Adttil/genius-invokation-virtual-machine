[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **dice_removed**

# givm::dice_removed

定义于头文件 `<givm/definition.hpp>`

```cpp
struct dice_removed;
```

元素骰移除或支付后的通知，由 [`remove_dice`](../commands/remove_dice.md) 或行动支付发送。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `const player_id` | 这次事件对应的玩家；只读 |
| `dice` | `const dice_counts` | 本次失去或支付的骰子及数量；只读 |

## 注意

[`remove_dice`](../commands/remove_dice.md) 一次性扣除全部指定骰子后，全场广播一次本事件，再继续后续命令。`dice` 是本次扣除量，不是玩家扣除后的骰子总量；全部数量为零时不广播。动态命令通过本事件的别名 [`remove_dice_input`](../command_inputs/remove_dice_input.md) 提交参数。

行动支付先完成骰子与充能扣除，再处理非零骰子支付的本事件，随后处理非零充能支付的 [`energy_changed`](energy_changed.md)。

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

## 参阅

| | |
| --- | --- |
| [`remove_dice`](../commands/remove_dice.md) | 扣除指定元素骰的命令 |
| [`dice_counts`](../../enums/dice_counts.md) | 各类元素骰的数量 |
