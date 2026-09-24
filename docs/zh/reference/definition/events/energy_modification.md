[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **energy_modification**

# givm::energy_modification

定义于头文件 `<givm/definition.hpp>`

```cpp
struct energy_modification
{
    character_id target;
    std::int64_t delta{};
};
```

[`modify_energy`](../commands/modify_energy.md) 的动态输入，包含目标角色和本次充能增量。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `character_id` | 要修改的有效角色实体，允许生命值为零 |
| `delta` | `std::int64_t` | 对充能的有符号增量，正数增加、负数减少，默认零 |

## 注意

增量作用于命令实际执行时的当前充能；结果限制在零和角色当前 `max_energy` 之间，包括 `INT64_MIN`、`INT64_MAX` 在内的增量都不会导致算术回绕。

本类型仅作为命令输入，不是可订阅的通知。命令不改变充能类型，也不发送充能变化广播。
