[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **add_combat_status_input**

# givm::add_combat_status_input

定义于头文件 `<givm/definition/commands.hpp>`

[add_combat_status](../commands/add_combat_status.md) 的动态输入，指定本次出战状态的目标、定义和状态。

```cpp
struct add_combat_status_input
{
    player_id player;
    definition_id<combat_status_view> definition;
    combat_status_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `player_id` | 目标玩家 |
| `definition` | `definition_id<combat_status_view>` | 本次使用的定义 |
| `state` | `combat_status_state` | 本次请求的状态，各字段默认 `UINT32_MAX`；命令执行时裁剪至定义上限 |

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。
