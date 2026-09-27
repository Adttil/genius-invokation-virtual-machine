[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_attachment_state**

# givm::set_attachment_state

定义于头文件 `<givm/definition.hpp>`

设置一个角色附属实体的状态。用于指定层数与本回合剩余次数。

```cpp
struct set_attachment_state
{
    using input_type = set_attachment_state_input;

    relative_attachment_target target{};
    attachment_state state{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`set_attachment_state_input`](../command_inputs/set_attachment_state_input.md)，动态模式下的输入类型 |

## 输入

- 默认构造 `set_attachment_state{}` 使用动态模式，由 `invoke` 提交一个 [set_attachment_state_input](../command_inputs/set_attachment_state_input.md)。
- 在 `target.selector` 中显式指定定义或装备类别时使用固定模式，不消费响应输入。按 `target.character` 的相对位置定位有效角色，不跳过生命值为零的角色，再选取此角色上首个同定义附属实体或当前指定类别装备。

动态输入可使用具体附属实体 ID，也可使用角色 ID 与装备类别，在命令执行时定位该角色当前的装备。两种模式均要求相应实体存在；完整定位规则见 [附属实体定位](attachment_target.md)。

## 结算

执行时读取 [attachment_state_limit](../queries/attachment_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。然后以裁剪结果替换目标的完整状态。`state{}` 的两个字段均为零。

先写入新状态，再仅向被修改的实体发送 [attachment_state_changed](../events/attachment_state_changed.md)。层数或本回合次数为零时是否离场，由定义在此响应中决定。

通知包含修改前和裁剪后的状态；返回的响应程序完整结算后才继续下一条命令。
