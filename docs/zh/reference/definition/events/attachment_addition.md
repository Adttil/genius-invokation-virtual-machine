[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **attachment_addition**

# givm::attachment_addition

定义于头文件 `<givm/definition.hpp>`

为一名角色添加附属实体或装备所需的初始信息。响应通过 `invoke` 提交它，由 [add_attachment](../commands/add_attachment.md) 消费；它本身不进行广播。

```cpp
struct attachment_addition
{
    character_id target;
    definition_id<attachment_view> definition;
    attachment_state state;
};
```

| 成员 | 说明 |
| --- | --- |
| `target` | 接受附属实体的角色，须为有效角色 |
| `definition` | 要创建的 attachment 定义 |
| `state` | 新实体的初始状态 |

装备类别由 attachment 定义的 `weapon`、`artifact`、`talent` 或 `technique` 标签决定，没有这些标签时为普通附属实体；四种标签互斥。武器匹配、角色归属、存活等用牌条件由定义的目标验证负责，添加命令不会代替调用方检查这些条件。

普通附属实体独立追加；装备会先让同类旧装备离场，再创建新装备。目标和状态在响应提交时确定，不需要命令读取外层事件。
