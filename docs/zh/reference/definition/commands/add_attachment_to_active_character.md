[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **add_attachment_to_active_character**

# givm::add_attachment_to_active_character

定义于头文件 `<givm/definition.hpp>`

为指定一方执行时的出战角色添加附属实体或装备。定义和初始状态保存在固定程序中，无需响应提交输入。

```cpp
struct add_attachment_to_active_character
{
    using input_type = void;

    relative_player player = relative_player::current;
    definition_id<attachment_view> definition;
    attachment_state state;
};
```

`player` 相对于当前行动玩家，沿用 [relative_player](relative_player.md) 的含义，目标方必须有有效的出战角色。其余字段及替换、广播行为与 [add_attachment](add_attachment.md) 相同。

本命令开始执行时才确定出战角色，不自动使用响应者所属的一方。
