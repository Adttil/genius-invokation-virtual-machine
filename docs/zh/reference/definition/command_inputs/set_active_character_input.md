[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **set_active_character_input**

# givm::set_active_character_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
using set_active_character_input = active_character_changed;
```

切换出战角色的动态输入。目标须是有效存活角色；重复选择当前角色无效果。 配合 [`set_active_character`](../commands/set_active_character.md) 和 [`handle_context::invoke`](../../executor/handle_context/invoke.md) 使用。

## 注意

本类型是 [`active_character_changed`](../events/active_character_changed.md) 的别名，成员及其限定与该事件相同。`set_active_character::input_type` 指向此类型，响应可以用别名或原事件类型构造输入；将它用于输入不会单独触发广播，结算与通知仍由命令决定。
