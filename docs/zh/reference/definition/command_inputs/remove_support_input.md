[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **remove_support_input**

# givm::remove_support_input

定义于头文件 `<givm/definition/commands.hpp>`

[remove_support](../commands/remove_support.md) 的动态输入，指定要离场的支援。

```cpp
struct remove_support_input
{
    support_id support;
};
```

`support` 必须指向有效实体。命令移除实体后，广播 [support_removed](../events/support_removed.md)。
