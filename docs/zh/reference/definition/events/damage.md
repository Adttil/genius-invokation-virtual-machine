[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage**

# givm::damage

定义于头文件 `<givm/definition.hpp>`

一次伤害的初始描述。由 [`deal_damage`](../commands/deal_damage.md) 使用，可在编译时指定，也可由响应通过 `invoke` 提交。它本身不广播；命令确定具体角色后，才开始 [`damage_preparation`](damage_preparation.md) 属性修饰和随后的数值计算。

```cpp
struct damage;
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`damage_source_id`](damage_source_id.md) | 造成伤害的来源 |
| `target` | [`damage_target`](damage_target.md) | 具体角色、相对出战位置或其余角色范围 |
| `value` | `std::uint32_t` | 初始伤害值 |
| `multiplier_numerator` | `std::uint16_t` | 初始倍率分子，默认为 1 |
| `multiplier_denominator` | `std::uint16_t` | 初始倍率的非零分母，默认为 1 |
| `type` | [`damage_type`](../../enums/damage_type.md) | 伤害种类 |
| `flags` | [`damage_flags`](../../enums/damage_flags.md) | 伤害附加属性 |

## 目标

- [`character_id`](../../table/character_id.md)：指定角色，轮到此次伤害时若已移除或生命为 0，跳过此次伤害。
- [`relative_character_target`](relative_character_target.md)：在处理该描述时，以指定一方的当前出战角色为基准定位存活目标。
- [`other_characters_target`](other_characters_target.md)：按循环顺序对指定角色以外的存活角色分别造成伤害。

范围中的每个目标分别进行伤害计算，初始伤害值等参数相同，计算和扣血读取当前牌桌。因此前一次伤害的计算、护盾消耗和扣血可以影响后一次伤害；同组的伤害后响应则推迟到全部伤害完成以后。

`flags` 中的普通攻击、元素战技和元素爆发标志描述本段伤害的性质，可独立组合。命令不会根据 `source` 自动补充这些标志；来源为技能时也应由定义源明确提供本段需要的标志。
