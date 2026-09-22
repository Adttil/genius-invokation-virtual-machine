[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **summon_removal**

# givm::summon_removal

定义于头文件 `<givm/definition.hpp>`

[remove_summon](../commands/remove_summon.md) 的动态输入，指定要离场的召唤物。

```cpp
struct summon_removal
{
    summon_id summon;
};
```

`summon` 必须指向有效实体。命令移除实体后，广播 [summon_removed](summon_removed.md)。
