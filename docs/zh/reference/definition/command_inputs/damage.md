[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **damage**

# givm::damage

定义于头文件 `<givm/definition.hpp>`。

一次伤害的来源、目标和数值，用作 [`deal_damage_input`](deal_damage_input.md) 数组的元素。单个元素可以选择一个角色或一个角色范围。

```cpp
struct damage
{
    damage_source_id source;
    damage_target target;
    character_selection selection = character_selection::character;
    std::uint32_t value;
    std::uint16_t multiplier_numerator = 1;
    std::uint16_t multiplier_denominator = 1;
    damage_type type;
    damage_flags flags;
};
```

来源可以是牌、角色、技能、召唤物等实体 ID。`target` 为角色 ID 时，以该角色为锚点，使用本对象的 `selection`；默认 `character` 只命中这个 ID。相对目标使用 [`relative_character_target`](../events/relative_character_target.md) 自己的 `selection`。

普通目标不会自动转移到其他存活角色。同段零生命但仍 `alive` 的角色继续参与伤害计算和反应；确认击倒后跳过。明确使用 `prioritized` 才会从锚点开始按循环顺序寻找第一名 `alive && health > 0` 的角色，此选择不改变出战位置。

`value` 是基础伤害，倍率默认 1；分母不能为零。每个命中分别参与伤害修饰及反应，同段通知按目标合并。具体计算规则见 [`deal_damage`](../commands/deal_damage.md)。
