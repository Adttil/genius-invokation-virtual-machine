[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **attachment_removal**

# givm::attachment_removal

定义于头文件 `<givm/definition.hpp>`

指定要移除的附属实体或装备，由 [remove_attachment](../commands/remove_attachment.md) 消费；本身不进行广播。

```cpp
struct attachment_removal
{
    attachment_id attachment;
};
```

`attachment` 须指向尚未移除的实体。移除前广播 [entity_will_leave](entity_will_leave.md)，实际移除后广播 [entity_left](entity_left.md)。
