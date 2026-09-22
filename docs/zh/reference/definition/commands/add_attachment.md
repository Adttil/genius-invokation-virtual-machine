[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **add_attachment**

# givm::add_attachment

定义于头文件 `<givm/definition.hpp>`

直接添加一个角色附属实体。可用于定义希望独立保留多个同类效果，或在移除旧实体后创建新实体的情况。

```cpp
struct add_attachment
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

- 默认构造 `add_attachment{}` 使用动态模式，由 `invoke` 提交一个 [attachment_addition](../events/attachment_addition.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定一方执行到本命令时的出战角色。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前行动玩家。动态输入明确指定目标角色和定义，目标角色必须有效。固定模式的出战角色目标必须有效。

## 结算

若本次定义带 `control` 标签，且目标当前具有 `control_immunity` 附属，则忽略本次添加，不移除现有装备。保护不解除已有控制；相关查询见 [`definition_library::is_control_immune`](../../executor/definition_library/is_control_immune.md)。

执行时读取 [attachment_state_limit](../queries/attachment_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建较少层数或次数的实体。

直接创建独立实体，同定义实体的存在不改变本次操作。

attachment 定义的 `weapon`、`artifact`、`talent`、`technique` 标签分别表示武器、圣遗物、天赋和特技；没有这些标签时为普通附属实体。这些类别标签互斥，由定义源保证。

- 普通附属实体直接追加，同一定义可有多个独立实体。
- 装备类别已被占用时，先移除旧装备，再广播 [attachment_removed](../events/attachment_removed.md)。
- 离场响应若另行安装同类装备，会继续移除当前占用者，完成其离场响应后再创建本次装备。

若旧装备离场响应使目标获得免控保护，而本次新装备带 `control` 标签，则停止后续添加；已经完成的旧装备移除不撤销。

同类装备至多保留一个。武器类型用 `sword`、`claymore`、`polearm`、`bow` 或 `catalyst` 标签表示，这些标签互斥；角色状态提供允许的武器类型掩码，实际用牌条件由卡牌定义决定。

角色的 `attachments()` 按加入顺序遍历，包括装备。通常广播中，同一角色的附属实体依次按武器、圣遗物、天赋、特技、普通附属实体响应；普通附属实体保持加入顺序。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。
