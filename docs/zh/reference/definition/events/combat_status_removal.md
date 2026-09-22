[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **combat_status_removal**

# givm::combat_status_removal

定义于头文件 `<givm/definition.hpp>`

[remove_combat_status](../commands/remove_combat_status.md) 的动态输入，指定要离场的出战状态。

```cpp
struct combat_status_removal
{
    combat_status_id status;
};
```

`status` 必须指向有效实体。命令移除实体后，广播 [combat_status_removed](combat_status_removed.md)。
