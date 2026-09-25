[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_combat_status_state**

# givm::set_combat_status_state

定义于头文件 `<givm/definition.hpp>`

设置一个出战状态的状态。用于指定层数与本回合剩余次数。

```cpp
struct set_combat_status_state
{
    using input_type = set_combat_status_state_input;

    relative_player player = relative_player::self;
    definition_id<combat_status_view> definition{};
    combat_status_state state{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`set_combat_status_state_input`](../command_inputs/set_combat_status_state_input.md)，动态模式下的输入类型 |

## 输入

- 默认构造 `set_combat_status_state{}` 使用动态模式，由 `invoke` 提交一个 [set_combat_status_state_input](../command_inputs/set_combat_status_state_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的出战状态中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入直接指定要操作的有效实体。

## 结算

执行时读取 [combat_status_state_limit](../queries/combat_status_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。然后以裁剪结果替换目标的完整状态。`state{}` 的两个字段均为零。

先写入新状态，再仅向被修改的实体发送 [combat_status_state_changed](../events/combat_status_state_changed.md)。层数或本回合次数为零时是否离场，由定义在此响应中决定。

通知包含修改前和裁剪后的状态；返回的响应程序完整结算后才继续下一条命令。
