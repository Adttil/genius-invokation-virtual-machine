[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **relative_character_target**

# givm::relative_character_target

定义于头文件 `<givm/definition.hpp>`

先以当前效果的本方或对方确定玩家，再以该玩家的当前出战角色为基准定位角色。在命令开始处理相应效果时确定具体角色，不在编译命令或响应提交输入时绑定实体 ID。

```cpp
struct relative_character_target
{
    relative_player player = relative_player::self;
    std::int32_t offset = 0;
    character_selection selection = character_selection::character;
};
```

| 成员 | 说明 |
| --- | --- |
| `player` | [`relative_player`](../commands/relative_player.md)，相对于当前效果本方的一方，默认 `self` |
| `offset` | 相对于当前出战角色的有符号位置偏移 |
| `selection` | [`character_selection`](../commands/character_selection.md)，以定位角色为基准选择 `character`、`others` 或 `all`；不需要范围时使用 `character` |

## 定位

玩家先按 [`table_state::self_player`](../../table/table_state.md) 确定，必须存在有效本方；动态伤害和治疗需要不依赖本方时，可以直接提供具体 `character_id`。随后以所选一方的当前出战角色为基准进行循环偏移：`0` 表示出战位置，`1` 表示下一个，`-1` 表示上一个。偏移按位置计算，不表示跳过若干名存活角色；目标方没有出战角色时无法定位。

伤害和切换出战角色会从偏移后的位置继续向后循环寻找存活角色；没有存活角色时跳过相应效果。作为基准的出战角色生命已经为 `0` 不妨碍继续定位其他角色。

## 来源和治疗

固定参数中的角色来源允许是已战败但未离场的角色；单角色治疗、增加生命上限、直接附着元素的目标同样允许生命值为 `0`。它们不会因为目标战败而改为另一个存活角色，因此濒死响应仍可通过单角色治疗复活原目标。

[`set_energy`](../commands/set_energy.md)、[`modify_energy`](../commands/modify_energy.md) 与 [`set_skill_state`](../commands/set_skill_state.md) 也按原位置定位，不跳过已战败但未离场的角色；这三个命令只接受 `character` 范围。

范围治疗的 `others` 和 `all` 只选择存活角色。它们先从偏移后的位置向后循环找到存活角色，再从此位置开始按循环顺序处理，`others` 跳过此角色；轮到后续每个角色时判断其是否存活。

## 作用范围

固定伤害、动态伤害及治疗使用本类型作为目标时，均由本类型的 `selection` 确定范围。动态 [`damage`](damage.md) 使用精确角色 ID 时，才由其自身的 `selection` 成员确定范围。作为来源或传给只接受单角色的命令时，使用默认的 `character`。

参见 [`damage`](damage.md)、[`set_active_character`](../commands/set_active_character.md)、[`heal`](../commands/heal.md)。
