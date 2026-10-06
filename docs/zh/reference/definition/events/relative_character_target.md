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
| `selection` | [`character_selection`](../commands/character_selection.md)，选择单体或角色范围；伤害还支持明确的 `prioritized` 优先目标 |

## 定位

玩家先按 [`table_state::self_player`](../../table/table_state.md) 确定，必须存在有效本方；动态伤害和治疗需要不依赖本方时，可以直接提供具体 `character_id`。随后以所选一方的当前出战角色为基准进行循环偏移：`0` 表示出战位置，`1` 表示下一个，`-1` 表示上一个。偏移按位置计算，不表示跳过若干名存活角色；目标方没有出战角色时无法定位。

普通伤害不跳过指定位置改选其他角色。只有伤害的 `prioritized` 从偏移后的位置继续向后循环寻找第一名 `alive && health > 0` 的角色；没有候选时无操作。切换出战角色继续使用其寻找存活角色的规则。作为基准的出战角色生命已经为 `0` 不妨碍定位其他位置。

[`transfer_attachment`](../commands/transfer_attachment.md) 的固定模式也从原角色和目标角色各自的偏移位置向后循环寻找存活角色，只接受 `character` 范围；调用方必须保证能定位到不同的角色，且原角色上存在相应附属实体。

## 来源和治疗

固定参数中的角色来源允许是已战败但未离场的角色；单角色治疗、增加生命上限、直接附着元素的目标同样允许生命值为 `0`。它们不会因为目标战败而改为另一个存活角色，因此濒死响应仍可通过单角色治疗复活原目标。

[`set_energy`](../commands/set_energy.md) 与 [`set_skill_state`](../commands/set_skill_state.md) 也按原位置定位，不跳过已战败但未离场的角色；这两个命令只接受 `character` 范围。

[`modify_energy`](../commands/modify_energy.md) 的 `character` 同样允许已战败但未离场的角色；`others` 和 `all` 只修改有效存活角色。它的范围锚点始终是循环偏移后的原位置，不会因锚点角色战败而顺延。

附属实体的设置状态、按增量修改和移除命令通过 [`relative_attachment_target`](../commands/attachment_target.md) 使用角色位置，同样不跳过生命值为零但仍有效的角色，只接受 `character` 范围。相应附属实体必须存在，因此濒死响应可以操作该角色尚未移除的附属实体。

范围伤害和治疗都以原始定位位置为锚点，`others` 排除这个角色，`all` 从这个位置开始循环。目标集合在每次操作开始时确定，中途复活或新增角色不扩充这次范围。普通范围排除已确认击倒者，复苏治疗范围可以包含已击倒者；实际治疗仍按治疗种类判断资格。

## 作用范围

固定伤害、动态伤害、治疗及固定模式的充能增量修改使用本类型作为目标时，均由本类型的 `selection` 确定范围。动态 [`damage`](../command_inputs/damage.md) 使用角色 ID 时，由其自身的 `selection` 成员确定范围。作为来源或传给只接受单角色的命令时，使用默认的 `character`；`prioritized` 仅用于伤害。

参见 [`damage`](../command_inputs/deal_damage_input.md)、[`set_active_character`](../commands/set_active_character.md)、[`heal`](../commands/heal.md)。
