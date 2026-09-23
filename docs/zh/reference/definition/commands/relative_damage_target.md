[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **relative_damage_target**

# givm::relative_damage_target

定义于头文件 `<givm/definition.hpp>`。

[`fixed_damage`](fixed_damage.md) 的目标描述，提供相对于出战角色的位置和伤害范围。可以命中一个角色、该角色以外的其他角色，或全部角色。

```cpp
struct relative_damage_target
{
    relative_player player = relative_player::opponent;
    std::int32_t offset = 0;
    damage_target_selection selection = damage_target_selection::character;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`relative_player`](relative_player.md) | 相对于当前效果本方的一方，默认为 `opponent` |
| `offset` | `std::int32_t` | 相对于当前出战位置的有符号偏移，默认为 `0` |
| `selection` | [`damage_target_selection`](damage_target_selection.md) | 伤害作用范围，默认为 `character` |

## 定位与展开

处理本段描述时，先根据 [`table_state::self_player`](../../table/table_state.md) 选择本方或对方，再以该玩家当时的出战位置为起点循环偏移。没有有效本方时使用相对目标属于未定义行为。`0` 表示出战位置，`1` 表示下一个，`-1` 表示上一个；若落在战败角色，继续向后循环寻找存活角色。没有出战角色或没有存活角色时，该段不产生伤害。

定位到存活角色后，先对本段描述广播一次 [`damage_preparation`](../events/damage_preparation.md)。响应可以修改定位角色及伤害属性，之后才按 [`damage_target_selection`](damage_target_selection.md) 选择范围。`character` 只选择修饰后的角色；`others` 从其下一个位置开始，按循环顺序选择其余存活角色；`all` 先选择此角色，再选择其余存活角色，每个角色只选择一次。因此，没有响应改动目标时，出战角色已战败且偏移为 `0` 的 `others` 排除的是向后找到的第一个存活角色。

范围在准备阶段确定，之后复活的其他角色不会加入本次范围。展开的伤害复制本段修饰后的来源、元素、数值、倍率和标志，不分别广播属性修饰事件。各具体目标仍分别判定反应、计算数值和扣除生命，且属于同一伤害组。扣血阶段目标若已离场或生命为零，跳过对它的伤害，但不会撤销此前已经准备的反应派生伤害。
