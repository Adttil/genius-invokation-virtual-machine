[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **support_removal**

# givm::support_removal

定义于头文件 `<givm/definition.hpp>`

[remove_support](../commands/remove_support.md) 的动态输入，指定要离场的支援。

```cpp
struct support_removal
{
    support_id support;
};
```

`support` 必须指向有效实体。命令移除实体后，广播 [support_removed](support_removed.md)。
