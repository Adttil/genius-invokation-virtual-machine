[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_attachment**

# givm::remove_attachment

定义于头文件 `<givm/definition.hpp>`

使一个角色附属实体离场，并通知其他实体处理相应效果。

```cpp
struct remove_attachment
{
    using input_type = remove_attachment_input;

    relative_attachment_target target{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`remove_attachment_input`](../command_inputs/remove_attachment_input.md)，动态模式下的输入类型 |

## 输入

- 默认构造 `remove_attachment{}` 使用动态模式，由 `invoke` 提交一个 [remove_attachment_input](../command_inputs/remove_attachment_input.md)。
- 在 `target.selector` 中显式指定定义或装备类别时使用固定模式，不消费响应输入。按 `target.character` 的相对位置定位有效角色，不跳过生命值为零的角色，再选取此角色上首个同定义附属实体或当前指定类别装备。

动态输入可使用具体附属实体 ID，也可使用角色 ID 与装备类别，在命令执行时定位该角色当前的装备。两种模式均要求相应实体存在；完整定位规则见 [附属实体定位](attachment_target.md)。

## 结算

移除指定实体，再广播 [attachment_removed](../events/attachment_removed.md)，完整结算离场响应后继续下一条命令。实体离场后不再参与通常的遍历和广播；在 cleanup 前，其定义和状态仍可由旧 ID 读取。
