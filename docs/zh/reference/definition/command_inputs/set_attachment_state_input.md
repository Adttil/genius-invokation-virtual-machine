[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **set_attachment_state_input**

# givm::set_attachment_state_input

定义于头文件 `<givm/definition/commands.hpp>`

[set_attachment_state](../commands/set_attachment_state.md) 的动态输入，指定要修改的角色附属实体以及本次变化。

```cpp
struct set_attachment_state_input
{
    attachment_id attachment;
    attachment_state state;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `attachment` | `attachment_id` | 要修改的有效实体 |
| `state` | `attachment_state` | 要设置的完整状态，默认两个字段均为零；执行时裁剪至定义上限 |
