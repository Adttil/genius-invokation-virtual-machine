[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **modify_summon_state_input**

# givm::modify_summon_state_input

定义于头文件 `<givm/definition/commands.hpp>`

[modify_summon_state](../commands/modify_summon_state.md) 的动态输入，指定要修改的召唤物以及本次变化。

```cpp
struct modify_summon_state_input
{
    summon_id summon;
    std::int64_t value{};
    std::int64_t usages{};
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `summon` | `summon_id` | 要修改的有效实体 |
| `value` | `std::int64_t` | 对同名状态字段的增量，默认零 |
| `usages` | `std::int64_t` | 对同名状态字段的增量，默认零 |

增量作用于命令实际执行时的当前值，结果饱和至零和定义上限之间。
