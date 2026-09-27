[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **remove_attachment_input**

# givm::remove_attachment_input

定义于头文件 `<givm/definition/commands.hpp>`

[remove_attachment](../commands/remove_attachment.md) 的动态输入，指定要离场的角色附属实体。

```cpp
struct remove_attachment_input
{
    attachment_target attachment;
};
```

[`attachment_target`](../commands/attachment_target.md) 可以指定有效实体 ID，也可以指定角色 ID 与装备类别，在执行时定位该角色当前的装备；相应实体必须存在。命令移除本次定位的实体后，广播 [attachment_removed](../events/attachment_removed.md)，不会继续移除通知响应中新加入的装备。
