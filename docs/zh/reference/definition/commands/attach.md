[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **attach**

# givm::attach

定义于头文件 `<givm/definition.hpp>`

向角色附属状态或装备；已有同定义实体时，由它决定本次请求如何影响现有效果。

```cpp
struct attach
{
    relative_player player = relative_player::current;
    definition_id<attachment_view> definition{};
    attachment_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 输入

- 默认构造 `attach{}` 使用动态模式，由 `invoke` 提交一个 [attachment_application](../events/attachment_application.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定一方执行到本命令时的出战角色。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前行动玩家。动态输入明确指定目标角色和定义，目标角色必须有效。固定模式的出战角色目标必须有效。

## 结算

若本次定义带 `control` 标签，且目标当前具有 `control_immunity` 附属，则忽略本次请求：不创建实体，也不调用已有同定义实体的重复施加响应。保护不移除已经存在的控制。查询保护可用 [`definition_library::is_control_immune`](../../executor/definition_library/is_control_immune.md)。

执行时读取 [attachment_state_limit](../queries/attachment_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建较少层数或次数的实体。

在目标范围查找首个有效、定义 ID 相同的实体：

- 没有匹配实体时，使用裁剪后的状态创建实体。装备替换遵守 [add_attachment](add_attachment.md) 的规则。
- 已有匹配实体时，仅向该实体发送 [attachment_reapplication](../events/attachment_reapplication.md)，携带本次裁剪后的状态；响应可从自身读取原有状态。
- 返回的响应程序完整结算后，本命令结束。未提供响应或返回空入口时，不再追加实体。多个同定义实体也只通知首个。

已有实体的响应可以通过 [modify_attachment_state](modify_attachment_state.md)、[set_attachment_state](set_attachment_state.md)、[remove_attachment](remove_attachment.md) 或 [add_attachment](add_attachment.md) 表达累加、刷新、删除重建和独立创建。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。
