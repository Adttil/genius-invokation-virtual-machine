[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_attachment**

# givm::remove_attachment

定义于头文件 `<givm/definition.hpp>`

使一个角色附属实体离场，并通知其他实体处理相应效果。

```cpp
struct remove_attachment
{
    using input_type = remove_attachment_input;

    relative_player player = relative_player::self;
    definition_id<attachment_view> definition{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`remove_attachment_input`](../command_inputs/remove_attachment_input.md)，动态模式下的输入类型 |

## 输入

- 默认构造 `remove_attachment{}` 使用动态模式，由 `invoke` 提交一个 [remove_attachment_input](../command_inputs/remove_attachment_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定一方执行到本命令时的出战角色。在该出战角色的附属实体中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入直接指定要操作的有效实体。固定模式的出战角色目标必须有效。

## 结算

移除指定实体，再广播 [attachment_removed](../events/attachment_removed.md)，完整结算离场响应后继续下一条命令。实体离场后不再参与通常的遍历和广播；在 cleanup 前，其定义和状态仍可由旧 ID 读取。
