[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **set_summon_state_input**

# givm::set_summon_state_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
struct set_summon_state_input
{
    struct change
    {
        summon_id summon;
        summon_state state;
    };

    std::span<const change> changes;
};
```

批量设置召唤物状态的输入，每个目标可以采用不同的完整状态。 配合 [`set_summon_state`](../commands/set_summon_state.md) 使用。

## 成员类型

| | |
| --- | --- |
| `change` | 一个目标召唤物及其待写入状态 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `changes` | `std::span<const change>` | 本次全部状态设置项 |

## 注意

`changes` 按顺序指定目标及新状态；允许为空，目标不得重复。命令只按各定义上限裁剪并写入，不通知自身、不因次数归零而离场。

[`invoke`](../../executor/handle_context/invoke.md) 复制数组内容，返回后不再借用原数组。数组须在复制期间保持有效。目标须在命令开始执行时有效。
