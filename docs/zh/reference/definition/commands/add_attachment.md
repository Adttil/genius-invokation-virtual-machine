[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **add_attachment**

# givm::add_attachment

定义于头文件 `<givm/definition.hpp>`

为角色添加普通附属实体或装备，并完成同类装备的替换。目标可以由响应提供，也可以是指定一方执行时的出战角色。

```cpp
struct add_attachment
{
    relative_player player = relative_player::current;
    definition_id<attachment_view> definition{};
    attachment_state state{};
};
```

## 输入

通过构造命令选择参数的提供方式：

- 默认构造 `add_attachment{}` 使用动态输入，消费响应通过 `invoke` 提交的一个 [attachment_addition](../events/attachment_addition.md)，由输入提供目标、定义和初始状态。
- 显式指定 `definition` 时，使用命令中的固定 `definition`、`state`，为 `player` 指定一方的出战角色添加实体，不消费响应输入。`player` 相对于当前行动玩家，沿用 [relative_player](relative_player.md) 的含义；目标方必须有有效的出战角色。本命令开始执行时才确定目标，不自动使用响应者所属的一方。

## 结算

attachment 定义的 `weapon`、`artifact`、`talent`、`technique` 标签分别表示武器、圣遗物、天赋和特技；没有这些标签时为普通附属实体。四种类别标签互斥，由定义源保证，不进行冲突检查。

- 普通附属实体直接追加；同一定义重复添加也创建独立实体。
- 装备类别已有旧装备时，先广播旧实体的 [entity_will_leave](../events/entity_will_leave.md)，移除它，再广播 [entity_left](../events/entity_left.md)。
- 离场响应若另行安装同类装备，会继续按上述规则移除当前占用者，然后创建本次装备。
- 创建完成后广播 [attachment_added](../events/attachment_added.md)，随后继续程序。

旧装备在移除前仍可参与广播，移除后不再参与通常的遍历和广播，其信息在 cleanup 前仍可按旧 ID 读取。

目标、定义及装备条件由调用方保证合法；不会自动执行目标验证。武器类型由定义的 `sword`、`claymore`、`polearm`、`bow` 或 `catalyst` 标签表示，这五种类型标签互斥。角色状态提供允许的武器类型掩码，具体用牌条件仍由卡牌定义决定。

同类装备至多保留一个。普通附属实体没有在本命令中附加叠层、刷新或归零移除规则。

当前行动、费用和本命令的常规广播中，同一角色的附属实体依次按武器、圣遗物、天赋、特技、普通附属实体的顺序响应；普通附属实体保持加入顺序。角色的 `attachments()` 遍历仍保持加入顺序，包含装备。其他事件的广播范围和顺序由其具体流程决定。
