[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **modify_energy_input**

# givm::modify_energy_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
struct modify_energy_input
{
    std::span<const character_id> targets;
    std::int64_t delta{};
};
```

[`modify_energy`](../commands/modify_energy.md) 的动态输入，包含目标角色列表和共同使用的充能增量。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `targets` | `std::span<const character_id>` | 按处理顺序排列的目标角色 ID 列表 |
| `delta` | `std::int64_t` | 对充能的有符号增量，正数增加、负数减少，默认零 |

## 注意

`targets` 可以跨双方，允许为空。每个目标必须在命令执行时有效，允许生命值为零。列表按输入顺序逐项处理；重复 ID 表示对同一角色多次应用增量，每次都执行饱和处理。

[`invoke`](../../executor/handle_context/invoke.md) 复制数组内容，返回后不再借用原数组；数组须在复制期间保持有效。目标在提交时由定义选择，之后出战位置变化不会改变列表。例如 `givm::modify_energy_input{ .targets = targets, .delta = 1 }` 对 `targets` 中的每项各增加一点充能，单角色输入可使用只有一个 ID 的数组。

增量作用于命令实际执行时的当前充能；结果限制在零和角色当前 `max_energy` 之间，包括 `INT64_MIN`、`INT64_MAX` 在内的增量都不会导致算术回绕。

本类型仅作为命令输入，不是可订阅的通知。命令不改变或筛选充能类型，也不发送充能变化广播；普通充能与替代充能的目标筛选由定义负责。
