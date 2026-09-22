[givm](../reference.md) / **枚举值**

# 枚举值

元素、骰子、行动、装备和对局结果的枚举类型，以及相关的类和函数。

## 枚举

|  |  |
| --- | --- |
| [`action_speed`](enums/action_speed.md) | 行动的速度类别 |
| [`damage_flag_bits`](enums/damage_flag_bits.md) | 一次伤害可以附带的独立性质 |
| [`damage_type`](enums/damage_type.md) | 伤害的元素或特殊类别 |
| [`element`](enums/element.md) | 对局中的元素种类，可用于描述元素附着、伤害带来的元素以及元素反应 |
| [`element_application_cause`](enums/element_application_cause.md) | 本次施加元素的来源，区分独立效果施加元素与伤害附带的元素 |
| [`element_aura`](enums/element_aura.md) | 角色身上保留的元素附着 |
| [`elemental_reaction`](enums/elemental_reaction.md) | 元素相遇时产生的反应种类 |
| [`equipment_type`](enums/equipment_type.md) | 附着实体定义的装备类别 |
| [`game_result`](enums/game_result.md) | 一场对局的结果，区分尚未结束、某一方获胜和双方均告负 |
| [`weapon_type`](enums/weapon_type.md) | 武器类别 |
| [`elemental_dice`](enums/elemental_dice.md) | 元素骰的种类，包括七种元素和万能元素 |

## 类

|  |  |
| --- | --- |
| [`dice_counts`](enums/dice_counts.md) | 各种元素骰的持有数量 |
| [`elemental_dice_cost`](enums/elemental_dice_cost.md) | 各种元素骰的费用数量 |
| [`elemental_dice_requirement`](enums/elemental_dice_requirement.md) | 指定、同色和任意元素骰的费用需求 |
| [`skill_flags`](enums/skill_flags.md) | 一次技能使用的性质组合 |
| [`skill_flag_bits`](enums/skill_flag_bits.md) | 技能使用的独立性质 |
| [`damage_flags`](enums/damage_flags.md) | 一次伤害的性质组合 |
| [`weapon_type_mask`](enums/weapon_type_mask.md) | 允许装备的武器类别集合 |

## 函数

|  |  |
| --- | --- |
| [`element_from_damage_type`](enums/element_from_damage_type.md) | 取得伤害所带的元素 |
| [`aura_from_element`](enums/aura_from_element.md) | 取得单元素附着 |
| [`primary_element_from_aura`](enums/primary_element_from_aura.md) | 取得优先参与反应的附着元素 |
| [`aura_without_reaction`](enums/aura_without_reaction.md) | 取得不发生反应时的附着 |
| [`reaction_between`](enums/reaction_between.md) | 判断两个元素的反应 |
| [`reaction_from_aura`](enums/reaction_from_aura.md) | 判断附着与新元素的反应 |
| [`aura_after_reaction`](enums/aura_after_reaction.md) | 取得反应后的附着 |
| [`elemental_dice_from_random`](enums/elemental_dice_from_random.md) | 从随机值取得骰子种类 |
