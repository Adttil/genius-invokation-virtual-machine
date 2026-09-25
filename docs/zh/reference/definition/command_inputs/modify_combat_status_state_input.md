[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **modify_combat_status_state_input**

# givm::modify_combat_status_state_input

定义于头文件 `<givm/definition/commands.hpp>`

[modify_combat_status_state](../commands/modify_combat_status_state.md) 的动态输入，指定要修改的出战状态以及本次变化。

```cpp
struct modify_combat_status_state_input
{
    combat_status_id status;
    std::int64_t count{};
    std::int64_t round_usages{};
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `status` | `combat_status_id` | 要修改的有效实体 |
| `count` | `std::int64_t` | 对同名状态字段的增量，默认零 |
| `round_usages` | `std::int64_t` | 对同名状态字段的增量，默认零 |

增量作用于命令实际执行时的当前值，结果饱和至零和定义上限之间。
