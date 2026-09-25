[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_combat_status**

# givm::remove_combat_status

定义于头文件 `<givm/definition.hpp>`

使一个出战状态离场，并通知其他实体处理相应效果。

```cpp
struct remove_combat_status
{
    using input_type = remove_combat_status_input;

    relative_player player = relative_player::self;
    definition_id<combat_status_view> definition{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`remove_combat_status_input`](../command_inputs/remove_combat_status_input.md)，动态模式下的输入类型 |

## 输入

- 默认构造 `remove_combat_status{}` 使用动态模式，由 `invoke` 提交一个 [remove_combat_status_input](../command_inputs/remove_combat_status_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的出战状态中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入直接指定要操作的有效实体。

## 结算

移除指定实体，再广播 [combat_status_removed](../events/combat_status_removed.md)，完整结算离场响应后继续下一条命令。实体离场后不再参与通常的遍历和广播；在 cleanup 前，其定义和状态仍可由旧 ID 读取。
