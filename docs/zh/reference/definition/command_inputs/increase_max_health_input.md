[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **increase_max_health_input**

# givm::increase_max_health_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
using increase_max_health_input = healing;
```

增加生命上限的动态输入。命令先增加上限，再恢复实际增加量的生命并发送治疗完成通知。 配合 [`increase_max_health`](../commands/increase_max_health.md) 和 [`handle_context::invoke`](../../executor/handle_context/invoke.md) 使用。

## 注意

本类型是 [`healing`](../events/healing.md) 的别名，成员及其限定与该事件相同。`increase_max_health::input_type` 指向此类型，响应可以用别名或原事件类型构造输入；将它用于输入不会单独触发广播，结算与通知仍由命令决定。
