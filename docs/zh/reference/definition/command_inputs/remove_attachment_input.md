[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **remove_attachment_input**

# givm::remove_attachment_input

定义于头文件 `<givm/definition/commands.hpp>`

[remove_attachment](../commands/remove_attachment.md) 的动态输入，指定要离场的角色附属实体。

```cpp
struct remove_attachment_input
{
    attachment_id attachment;
};
```

`attachment` 必须指向有效实体。命令移除实体后，广播 [attachment_removed](../events/attachment_removed.md)。
