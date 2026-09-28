[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **remove_dice_input**

# givm::remove_dice_input

定义于头文件 `<givm/definition.hpp>`

```cpp
using remove_dice_input = dice_removed;
```

移除元素骰命令的动态输入。命令一次性扣除指定玩家的骰子，再发送移除完成通知。配合 [`remove_dice`](../commands/remove_dice.md) 和 [`handle_context::invoke`](../../executor/handle_context/invoke.md) 使用。

## 注意

本类型是 [`dice_removed`](../events/dice_removed.md) 的别名，成员及其限定与该事件相同。`remove_dice::input_type` 指向此类型，响应可以用别名或原事件类型构造输入；将它用于输入不会单独触发广播，结算与通知仍由命令决定。

输入的玩家 ID 必须有效，命令执行时玩家每类骰子的数量必须足够扣除。
