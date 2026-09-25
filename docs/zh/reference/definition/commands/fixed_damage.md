[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **fixed_damage**

# givm::fixed_damage

定义于头文件 `<givm/definition.hpp>`。

[`deal_damage`](deal_damage.md) 的固定伤害描述。它可以组成固定长度的伤害组，不包含对局实体 ID。

```cpp
struct fixed_damage
{
    relative_character_target source;
    relative_character_target target{ relative_player::opponent };
    std::uint32_t value;
    std::uint16_t multiplier_numerator = 1;
    std::uint16_t multiplier_denominator = 1;
    damage_type type;
    damage_flags flags;
};
```

## 成员对象

| 名称 | 说明 |
| --- | --- |
| `source` | [来源角色位置](../events/relative_character_target.md)，默认本方出战角色，允许尚未离场的战败角色 |
| `target` | [相对角色位置及作用范围](../events/relative_character_target.md)，默认对方出战角色 |
| `value` | 初始伤害值 |
| `multiplier_numerator` | 初始倍率分子，默认 1 |
| `multiplier_denominator` | 初始倍率的非零分母，默认 1 |
| `type` | 伤害元素或物理、穿透种类 |
| `flags` | 明确指定的伤害属性 |

处理每段描述时才解析来源和目标，找不到来源或存活目标时跳过该段。目标位置落在战败角色时，先循环向后寻找存活角色，再按范围选择该角色、其他角色或全部角色。范围目标按循环顺序逐个结算，仍属于同一伤害组；定位和展开规则见 [`relative_character_target`](../events/relative_character_target.md)。

本方由 [`table_state::self_player`](../../table/table_state.md) 确定，根流程使用固定伤害前须显式设置有效本方。需要精确的技能、召唤物或卡牌来源时，应通过 `invoke` 提交动态 [`damage`](../command_inputs/damage.md)；命令不会把来源隐式设为响应实体，也不读取外层事件。
