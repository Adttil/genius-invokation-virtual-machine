[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **transfer_attachment_input**

# givm::transfer_attachment_input

定义于头文件 `<givm/definition/commands.hpp>`

[`transfer_attachment`](../commands/transfer_attachment.md) 的动态输入，指定被转移的附属实体和接收它的角色。

```cpp
struct transfer_attachment_input
{
    attachment_target attachment;
    character_id target;
    bool reset_round_usages = false;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `attachment` | [`attachment_target`](../commands/attachment_target.md) | 要转移的有效实体 ID，或执行时按角色 ID 与装备类别定位的装备 |
| `target` | `character_id` | 有效、存活且不同于原所属角色的目标角色 |
| `reset_round_usages` | `bool` | 是否恢复本回合可用次数，默认保留原值 |

转移保留层数；需要恢复次数时，将 `round_usages` 设为 [attachment_state_limit](../queries/attachment_state_limit.md) 返回的对应值。目标原有装备的离场通知在本次转移和次数恢复之后进行，具体见命令的结算规则。
