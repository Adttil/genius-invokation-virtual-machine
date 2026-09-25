[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **remove_combat_status_input**

# givm::remove_combat_status_input

定义于头文件 `<givm/definition/commands.hpp>`

[remove_combat_status](../commands/remove_combat_status.md) 的动态输入，指定要离场的出战状态。

```cpp
struct remove_combat_status_input
{
    combat_status_id status;
};
```

`status` 必须指向有效实体。命令移除实体后，广播 [combat_status_removed](../events/combat_status_removed.md)。
