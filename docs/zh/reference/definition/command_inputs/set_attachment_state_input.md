[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **set_attachment_state_input**

# givm::set_attachment_state_input

定义于头文件 `<givm/definition.hpp>`

[set_attachment_state](../commands/set_attachment_state.md) 的动态输入，指定要修改的角色附属实体以及本次变化。

```cpp
struct set_attachment_state_input
{
    attachment_target attachment;
    attachment_state state;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `attachment` | [`attachment_target`](../commands/attachment_target.md) | 要修改的有效实体 ID，或执行时按角色 ID 与装备类别定位的装备 |
| `state` | `attachment_state` | 要设置的完整状态，默认两个字段均为零；执行时按命令的 `ignore_limit` 选项处理 |
