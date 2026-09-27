[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **modify_attachment_state**

# givm::modify_attachment_state

定义于头文件 `<givm/definition.hpp>`

按增量调整一个角色附属实体的状态。可表达消耗、增加和次数恢复。

```cpp
struct modify_attachment_state
{
    using input_type = modify_attachment_state_input;

    relative_attachment_target target{};
    std::int64_t count{};
    std::int64_t round_usages{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`modify_attachment_state_input`](../command_inputs/modify_attachment_state_input.md)，动态模式下的输入类型 |

## 输入

- 默认构造 `modify_attachment_state{}` 使用动态模式，由 `invoke` 提交一个 [modify_attachment_state_input](../command_inputs/modify_attachment_state_input.md)。
- 在 `target.selector` 中显式指定定义或装备类别时使用固定模式，不消费响应输入。按 `target.character` 的相对位置定位有效角色，不跳过生命值为零的角色，再选取此角色上首个同定义附属实体或当前指定类别装备。

动态输入可使用具体附属实体 ID，也可使用角色 ID 与装备类别，在命令执行时定位该角色当前的装备。两种模式均要求相应实体存在；完整定位规则见 [附属实体定位](attachment_target.md)。

## 结算

执行到命令时，分别读取目标各字段的当前值，加上对应的有符号增量，再将结果限制在零与 [attachment_state_limit](../queries/attachment_state_limit.md) 对应字段之间。负增量表示消耗，正增量表示增加；`INT64_MIN`、`INT64_MAX` 也按这一规则处理，不发生算术回绕。

先写入新状态，再仅向被修改的实体发送 [attachment_state_changed](../events/attachment_state_changed.md)。层数或本回合次数为零时是否离场，由定义在此响应中决定。

通知包含修改前和裁剪后的状态；返回的响应程序完整结算后才继续下一条命令。

例如，动态输入 `modify_attachment_state_input{ .attachment = target, .round_usages = -1 }` 会在本命令实际执行时扣除一次可用次数；另一个字段的增量默认是零。
