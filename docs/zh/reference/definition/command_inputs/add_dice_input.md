[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **add_dice_input**

# givm::add_dice_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
using add_dice_input = dice_added;
```

产骰命令的动态输入。命令一次性增加指定玩家的骰子，再发送增加完成通知。 配合 [`add_dice`](../commands/add_dice.md) 和 [`handle_context::invoke`](../../executor/handle_context/invoke.md) 使用。

## 注意

本类型是 [`dice_added`](../events/dice_added.md) 的别名，成员及其限定与该事件相同。`add_dice::input_type` 指向此类型，响应可以用别名或原事件类型构造输入；将它用于输入不会单独触发广播，结算与通知仍由命令决定。
