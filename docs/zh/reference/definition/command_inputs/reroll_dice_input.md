[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **reroll_dice_input**

# givm::reroll_dice_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
struct reroll_dice_input
{
    player_id player;
    std::uint32_t reroll_count = 1;
};
```

单方重投命令的动态输入，指定进行重投的玩家和允许的次数。配合 [`reroll_dice`](../commands/reroll_dice.md) 和 [`handle_context::invoke`](../../executor/handle_context/invoke.md) 使用。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 进行重投的玩家 ID，必须有效 |
| `reroll_count` | `std::uint32_t` | 最多可重投的次数，初始为 1 |

## 注意

每次要重投的具体骰子由命令执行期间的 [`execution_view<dice_reroll_selection>`](../../executor/execution_view/dice_reroll_selection.md) 输入。次数为零或该玩家当前没有骰子时，命令直接完成。
