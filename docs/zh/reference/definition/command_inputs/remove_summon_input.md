[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **remove_summon_input**

# givm::remove_summon_input

定义于头文件 `<givm/definition/commands.hpp>`

[remove_summon](../commands/remove_summon.md) 的动态输入，指定要离场的召唤物。

```cpp
struct remove_summon_input
{
    summon_id summon;
};
```

`summon` 必须指向有效实体。命令移除实体后，广播 [summon_removed](../events/summon_removed.md)。
