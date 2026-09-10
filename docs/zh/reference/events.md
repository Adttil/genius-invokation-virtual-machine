[givm](../reference.md) / **事件**

# 事件

事件描述对局中正在发生或已经完成的事情。实体定义可以响应相关事件，读取通知的数据，或在允许修改的时机调整即将生效的效果。

事件类型也限定其响应中能够使用的[指令](instructions.md)。这种限制表达响应发生的语境，例如抵挡伤害只能用于伤害结算的响应。每次触发可以选择响应实体的范围与顺序，不要求所有事件使用统一的广播顺序。

## 回合与投骰

| | |
| --- | --- |
| [`battle_started`](events/battle_started.md) | 对局首次进入战斗的通知 |
| [`action_phase_started`](events/action_phase_started.md) | 本回合的行动阶段已经开始 |
| [`before_action`](events/before_action.md) | 当前行动玩家即将选择下一次行动 |
| [`round_end_declared`](events/round_end_declared.md) | 当前行动玩家已经宣布本回合结束 |
| [`round_ended`](events/round_ended.md) | 双方均已宣布结束，本回合已经关闭 |
| [`dice_roll_preparation`](events/dice_roll_preparation.md) | 本回合开始投骰前的准备事件 |
| [`dice_added`](events/dice_added.md) | 玩家已经获得一组元素骰的通知 |
| [`dice_removed`](events/dice_removed.md) | 玩家已经失去或支付一组元素骰的通知 |
| [`dice_converted`](events/dice_converted.md) | 玩家的一部分元素骰已经转换种类的通知 |

## 资源与费用

| | |
| --- | --- |
| [`changing_secret_points`](events/changing_secret_points.md) | 玩家即将增加或减少秘传点数 |
| [`secret_points_changed`](events/secret_points_changed.md) | 玩家的秘传点数已经改变的通知 |
| [`changing_energy`](events/changing_energy.md) | 角色即将获得或失去充能 |
| [`energy_changed`](events/energy_changed.md) | 角色的充能已经改变的通知 |
| [`calculating_card_payment`](events/calculating_card_payment.md) | 正在计算打出一张手牌所需的支付内容 |
| [`calculating_skill_payment`](events/calculating_skill_payment.md) | 正在计算使用一个技能所需的支付内容 |
| [`calculating_switch_payment`](events/calculating_switch_payment.md) | 正在计算切换出战角色所需的支付内容 |
| [`cost_of_switch`](events/cost_of_switch.md) | 正在计算主动切换出战角色的费用 |
| [`elemental_dice_requirement`](events/elemental_dice_requirement.md) | 一项行动对元素骰的需求 |
| [`action_cost_requirement`](events/action_cost_requirement.md) | 执行一项行动需要满足的骰子费用，以及行动完成后是否交出行动权 |
| [`cost_effect_argument`](events/cost_effect_argument.md) | 一次费用调整效果记录的减费内容 |

## 卡牌与技能

| | |
| --- | --- |
| [`hand_card_created`](events/hand_card_created.md) | 一张手牌已经被创建的通知 |
| [`card_drawn`](events/card_drawn.md) | 一张牌已经从牌堆进入手牌的通知 |
| [`card_discarded`](events/card_discarded.md) | 一张手牌或牌堆中的牌被弃置的通知 |
| [`card_candidate_chosen`](events/card_candidate_chosen.md) | 玩家已经从候选牌中选择了一种牌定义的通知 |
| [`elemental_tuning_will_apply`](events/elemental_tuning_will_apply.md) | 一张手牌即将用于元素调和 |
| [`elemental_tuning_applied`](events/elemental_tuning_applied.md) | 元素调和已经完成的通知 |
| [`card_will_be_played`](events/card_will_be_played.md) | 玩家即将打出一张手牌 |
| [`card_played`](events/card_played.md) | 打出手牌的处理已经完成的通知，包括最终目标、行动速度和效果是否被取消 |
| [`skill_will_be_used`](events/skill_will_be_used.md) | 角色即将使用技能 |
| [`skill_used`](events/skill_used.md) | 使用技能的处理已经完成的通知 |

## 伤害、治疗与元素

| | |
| --- | --- |
| [`damage_calculation`](events/damage_calculation.md) | 一次伤害正在计算 |
| [`damage_effect`](events/damage_effect.md) | 一次伤害即将扣除角色生命 |
| [`after_damage`](events/after_damage.md) | 一次伤害已经扣除生命，并完成它所携带的元素附着及反应处理 |
| [`healing`](events/healing.md) | 角色即将恢复生命 |
| [`healed`](events/healed.md) | 角色的治疗已经完成的通知 |
| [`elemental_reaction_will_occur`](events/elemental_reaction_will_occur.md) | 附着元素即将与角色身上的元素发生反应 |
| [`after_elemental_reaction`](events/after_elemental_reaction.md) | 一次元素反应的处理已经结束 |

## 角色与实体

| | |
| --- | --- |
| [`active_character_changed`](events/active_character_changed.md) | 一名玩家的出战角色已经设置的通知 |
| [`character_initialization`](events/character_initialization.md) | 角色的初始状态正在准备 |
| [`character_will_be_defeated`](events/character_will_be_defeated.md) | 角色即将被击倒 |
| [`character_defeated`](events/character_defeated.md) | 角色已经被击倒的通知 |
| [`entity_will_leave`](events/entity_will_leave.md) | 一个实体即将离场 |
| [`entity_left`](events/entity_left.md) | 一个实体已经离场的通知 |
| [`entity_count_changed`](events/entity_count_changed.md) | 具有次数或层数的实体，其计数已经改变的通知 |

## 标识类型

| | |
| --- | --- |
| [`card_id`](events/card_id.md) | 可能位于手牌或牌堆中的牌标识 |
| [`card_target_id`](events/card_target_id.md) | 卡牌效果的目标标识 |
| [`damage_source_id`](events/damage_source_id.md) | 伤害的来源标识 |
| [`effect_source_id`](events/effect_source_id.md) | 治疗等效果的来源标识，能够表示牌、角色、技能以及其他场上实体 |
| [`element_application_source_id`](events/element_application_source_id.md) | 元素附着的来源标识，用来识别是哪一个角色或效果施加了本次元素 |
| [`entity_id`](events/entity_id.md) | 能够指代各类牌桌实体的标识，用于不限定实体种类的离场等通知 |
| [`counted_entity_id`](events/counted_entity_id.md) | 具有计数的实体标识，用于报告状态、支援和召唤物等对象的次数或层数变化 |

## 调试

| | |
| --- | --- |
| [`test_event`](events/test_event.md) | 用于检查实体是否正确收到事件的测试通知 |
