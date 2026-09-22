[givm](../../reference.md) / [定义](../definition.md) / **事件**

# 事件

事件描述对局中正在发生或已经完成的事情。实体定义可以响应相关事件，读取通知的数据，或在允许修改的时机调整即将生效的效果。

事件也可用作命令的初始输入，但是否广播由具体命令决定。每次触发可以选择响应实体的范围与顺序，不要求所有事件使用统一的广播顺序。

## 回合与投骰

| | |
| --- | --- |
| [`battle_started`](events/battle_started.md) | 对局首次进入战斗的通知 |
| [`action_phase_started`](events/action_phase_started.md) | 本回合行动阶段开始的通知 |
| [`before_action`](events/before_action.md) | 当前行动玩家选择行动前的事件 |
| [`round_end_declared`](events/round_end_declared.md) | 玩家宣布本回合结束的通知 |
| [`round_ended`](events/round_ended.md) | 本回合结束的通知 |
| [`dice_roll_preparation`](events/dice_roll_preparation.md) | 本回合投骰前的准备事件 |
| [`dice_added`](events/dice_added.md) | 元素骰增加后的通知 |
| [`dice_removed`](events/dice_removed.md) | 元素骰移除或支付后的通知 |
| [`dice_converted`](events/dice_converted.md) | 元素骰种类转换后的通知 |

## 资源与费用

| | |
| --- | --- |
| [`changing_secret_points`](events/changing_secret_points.md) | 秘传点数变化前的调整事件 |
| [`secret_points_changed`](events/secret_points_changed.md) | 秘传点数变化后的通知 |
| [`changing_energy`](events/changing_energy.md) | 角色充能变化前的调整事件 |
| [`energy_changed`](events/energy_changed.md) | 角色充能变化后的通知 |
| [`calculating_card_payment`](events/calculating_card_payment.md) | 打出手牌时的支付计算事件 |
| [`calculating_skill_payment`](events/calculating_skill_payment.md) | 使用技能时的支付计算事件 |
| [`calculating_switch_payment`](events/calculating_switch_payment.md) | 切换出战角色时的支付计算事件 |
| [`cost_of_switch`](events/cost_of_switch.md) | 主动切换出战角色的费用计算事件 |
| [`cost_of_skill`](events/cost_of_skill.md) | 技能使用的费用计算事件 |
| [`cost_of_card`](events/cost_of_card.md) | 出牌的费用计算事件 |
| [`elemental_dice_requirement`](../enums/elemental_dice_requirement.md) | 一项行动对元素骰的需求 |
| [`action_cost_requirement`](../table/action_cost_requirement.md) | 一项行动的骰子、充能费用与行动速度 |

## 卡牌与技能

| | |
| --- | --- |
| [`hand_card_created`](events/hand_card_created.md) | 新手牌创建后的通知 |
| [`card_drawn`](events/card_drawn.md) | 一张牌抽取完成后的通知 |
| [`card_discarded`](events/card_discarded.md) | 手牌或牌堆中的牌被弃置后的通知 |
| [`card_candidate_chosen`](events/card_candidate_chosen.md) | 候选牌定义选定后的通知 |
| [`elemental_tuning_modification`](events/elemental_tuning_modification.md) | 修饰元素调和的转换结果 |
| [`elemental_tuning_completed`](events/elemental_tuning_completed.md) | 元素调和完成后的通知 |
| [`card_will_be_played`](events/card_will_be_played.md) | 手牌效果生效前的事件 |
| [`card_effect`](events/card_effect.md) | 执行打出的牌自身效果 |
| [`card_played`](events/card_played.md) | 打出手牌完成后的通知 |
| [`skill_will_be_used`](events/skill_will_be_used.md) | 技能效果生效前的事件 |
| [`skill_effect`](events/skill_effect.md) | 执行所选技能自身效果 |
| [`skill_used`](events/skill_used.md) | 技能使用完成后的通知 |

## 伤害、治疗与元素

| | |
| --- | --- |
| [`damage`](events/damage.md) | 单体或范围伤害的初始描述 |
| [`relative_character_target`](events/relative_character_target.md) | 相对于出战位置的伤害目标 |
| [`other_characters_target`](events/other_characters_target.md) | 指定角色以外的同方存活角色 |
| [`damage_calculation`](events/damage_calculation.md) | 伤害计算事件 |
| [`damage_effect`](events/damage_effect.md) | 扣除生命前的伤害结算事件 |
| [`after_damage`](events/after_damage.md) | 伤害及其元素附着结算完成后的通知 |
| [`healing`](events/healing.md) | 角色恢复生命前的治疗调整事件 |
| [`healed`](events/healed.md) | 角色治疗完成后的通知 |
| [`elemental_reaction_will_occur`](events/elemental_reaction_will_occur.md) | 元素反应生效前的事件 |
| [`after_elemental_reaction`](events/after_elemental_reaction.md) | 元素反应处理完成后的通知 |

## 角色与实体

| | |
| --- | --- |
| [`active_character_changed`](events/active_character_changed.md) | 出战角色设置完成后的通知 |
| [`character_will_be_defeated`](events/character_will_be_defeated.md) | 角色被击倒前的事件 |
| [`character_defeated`](events/character_defeated.md) | 角色被击倒后的通知 |
| [`summon_removed`](events/summon_removed.md) | 召唤物移除后向其他有效实体广播的通知 |
| [`combat_status_removed`](events/combat_status_removed.md) | 出战状态移除后向其他有效实体广播的通知 |
| [`attachment_removed`](events/attachment_removed.md) | 角色附属实体或装备移除后向其他有效实体广播的通知 |
| [`summoning`](events/summoning.md) | [summon](commands/summon.md) 的动态输入 |
| [`resummoning`](events/resummoning.md) | 仅向已有召唤物发送的重复请求 |
| [`summon_addition`](events/summon_addition.md) | [add_summon](commands/add_summon.md) 的动态输入 |
| [`summon_state_change`](events/summon_state_change.md) | [set_summon_state](commands/set_summon_state.md) 的动态输入 |
| [`summon_state_modification`](events/summon_state_modification.md) | [modify_summon_state](commands/modify_summon_state.md) 的动态输入 |
| [`summon_state_changed`](events/summon_state_changed.md) | 状态修改后的自身通知 |
| [`summon_removal`](events/summon_removal.md) | [remove_summon](commands/remove_summon.md) 的动态输入 |
| [`combat_status_generation`](events/combat_status_generation.md) | [generate_combat_status](commands/generate_combat_status.md) 的动态输入 |
| [`combat_status_regeneration`](events/combat_status_regeneration.md) | 仅向已有出战状态发送的重复请求 |
| [`combat_status_addition`](events/combat_status_addition.md) | [add_combat_status](commands/add_combat_status.md) 的动态输入 |
| [`combat_status_state_change`](events/combat_status_state_change.md) | [set_combat_status_state](commands/set_combat_status_state.md) 的动态输入 |
| [`combat_status_state_modification`](events/combat_status_state_modification.md) | [modify_combat_status_state](commands/modify_combat_status_state.md) 的动态输入 |
| [`combat_status_state_changed`](events/combat_status_state_changed.md) | 状态修改后的自身通知 |
| [`combat_status_removal`](events/combat_status_removal.md) | [remove_combat_status](commands/remove_combat_status.md) 的动态输入 |
| [`attachment_application`](events/attachment_application.md) | [attach](commands/attach.md) 的动态输入 |
| [`attachment_reapplication`](events/attachment_reapplication.md) | 仅向已有角色附属实体发送的重复请求 |
| [`attachment_addition`](events/attachment_addition.md) | [add_attachment](commands/add_attachment.md) 的动态输入 |
| [`attachment_state_change`](events/attachment_state_change.md) | [set_attachment_state](commands/set_attachment_state.md) 的动态输入 |
| [`attachment_state_modification`](events/attachment_state_modification.md) | [modify_attachment_state](commands/modify_attachment_state.md) 的动态输入 |
| [`attachment_state_changed`](events/attachment_state_changed.md) | 状态修改后的自身通知 |
| [`attachment_removal`](events/attachment_removal.md) | [remove_attachment](commands/remove_attachment.md) 的动态输入 |

## 标识类型别名

| | |
| --- | --- |
| [`card_id`](events/card_id.md) | 手牌或牌堆中的牌标识 |
| [`skill_target_id`](events/skill_target_id.md) | 技能效果的目标标识 |
| [`card_target_id`](events/card_target_id.md) | 卡牌效果的目标标识 |
| [`damage_source_id`](events/damage_source_id.md) | 伤害的来源标识 |
| [`damage_target`](events/damage_target.md) | 伤害的具体、相对或范围目标 |
| [`effect_source_id`](events/effect_source_id.md) | 治疗等效果的来源标识 |
| [`element_application_source_id`](events/element_application_source_id.md) | 元素附着的来源标识 |

## 调试

| | |
| --- | --- |
| [`test_event`](events/test_event.md) | 事件响应的测试通知 |
