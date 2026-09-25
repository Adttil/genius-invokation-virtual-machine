[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **dice_added**

# givm::dice_added

定义于头文件 `<givm/definition.hpp>`

```cpp
struct dice_added
{
    const player_id player;
    const dice_counts dice;
};
```

元素骰增加完成后的通知，由 [`add_dice`](../commands/add_dice.md) 发送。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `const player_id` | 这次事件对应的玩家；只读 |
| `dice` | `const dice_counts` | 本次获得的骰子及数量；只读 |

## 注意

命令一次性增加全部指定骰子后，全场广播一次本事件，再继续后续命令。`dice` 是本次增加量，不是玩家增加后的骰子总量；全部数量为零时不广播。动态命令通过本事件的别名 [`add_dice_input`](../command_inputs/add_dice_input.md) 提交参数。

投骰和元素调和不发送此通知。

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

## 参阅

| | |
| --- | --- |
| [`add_dice`](../commands/add_dice.md) | 增加指定元素骰的命令 |
