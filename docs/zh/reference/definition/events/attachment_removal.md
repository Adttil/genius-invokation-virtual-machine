[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **attachment_removal**

# givm::attachment_removal

定义于头文件 `<givm/definition.hpp>`

[remove_attachment](../commands/remove_attachment.md) 的动态输入，指定要离场的角色附属实体。

```cpp
struct attachment_removal
{
    attachment_id attachment;
};
```

`attachment` 必须指向有效实体。命令移除实体后，广播 [attachment_removed](attachment_removed.md)。
