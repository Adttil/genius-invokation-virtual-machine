[givm](../../reference.md) / [定义](../definition.md) / **事件**

# 事件

事件描述对局中正在发生或已经完成的事情。实体定义可以响应相关事件，读取通知的数据，或在允许修改的时机调整即将生效的效果。

事件用于实体响应；动态命令通过其声明的 [命令输入](command_inputs.md) 提交参数。输入类型可以显式复用字段相符的事件类型，复用本身不会增加广播。每次触发可以选择响应实体的范围与顺序，不要求所有事件使用统一的广播顺序。

## 全场广播

核心命令采用全场广播时，先处理牌桌 `active_player` 指定的当前行动玩家，再处理对方。每方依次处理：

1. 出战角色的技能与附属实体。
2. 出战状态。
3. 从出战角色的下一位置开始，按循环顺序处理其余角色的技能与附属实体。
4. 召唤物、支援、手牌及其附着状态、牌堆中的牌及其附着状态。

每名角色先处理技能，再依次处理武器、圣遗物、天赋、特技，最后处理普通附属实体。角色不订阅事件；被动能力通过角色持有的技能参与。没有出战角色时，先处理出战状态，再按角色位置处理各角色的技能与附属实体。召唤物、支援、出战状态与普通附属实体按加入顺序处理；手牌和牌堆使用各自公开遍历顺序，每张牌后紧接其附着状态。

一次广播开始时确定候选响应者及顺序。之后新建的实体不加入本次广播；已在候选中但随后失效的实体在轮到时跳过。各响应在轮到时读取当前牌桌与事件，前面的响应及其效果可能影响后面的判断。后续广播重新确定自己的候选响应者。

具体命令可以规定单实体响应或其他范围，例如 `skill_effect` 只交给所用技能。订阅能力见 [`subscribed_events`](subscribed_events.md)；具有响应能力不表示一定会参与每次广播。

## 回合与投骰

| | |
| --- | --- |
| [`battle_started`](events/battle_started.md) | 对局首次进入战斗的通知 |
| [`round_started`](events/round_started.md) | 投骰与全部重投完成后的新回合通知 |
| [`action_phase_started`](events/action_phase_started.md) | 本回合行动阶段开始的通知 |
| [`before_action`](events/before_action.md) | 当前行动玩家选择行动前的事件 |
| [`round_end_declared`](events/round_end_declared.md) | 玩家宣布本回合结束的通知 |
| [`round_ended`](events/round_ended.md) | 本回合结束的通知 |
| [`dice_roll_preparation`](events/dice_roll_preparation.md) | 本回合投骰前的准备事件 |
| [`dice_added`](events/dice_added.md) | 元素骰增加完成后的通知 |
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

抽牌成功时只广播 [`card_drawn`](events/card_drawn.md)，非抽牌方式加入手牌时使用 [`hand_card_added`](events/hand_card_added.md)。需要响应任意方式加入手牌的定义，应同时响应这两种事件；抽牌时它与仅响应抽牌的定义共同遵循同一次广播的响应顺序，不另设加入手牌的广播阶段。

| | |
| --- | --- |
| [`hand_card_added`](events/hand_card_added.md) | 非抽牌方式加入手牌后的通知 |
| [`card_drawn`](events/card_drawn.md) | 一张牌抽取完成后的通知 |
| [`hand_card_discard_effect`](events/hand_card_discard_effect.md) | 仅向手牌自身发送的舍弃效果事件 |
| [`deck_card_discard_effect`](events/deck_card_discard_effect.md) | 仅向牌堆牌自身发送的舍弃效果事件 |
| [`hand_card_discarded`](events/hand_card_discarded.md) | 手牌自身舍弃效果完成后的全场通知 |
| [`deck_card_discarded`](events/deck_card_discarded.md) | 牌堆牌自身舍弃效果完成后的全场通知 |
| [`card_candidate_chosen`](events/card_candidate_chosen.md) | 候选牌定义选定后的通知 |
| [`elemental_tuning_modification`](events/elemental_tuning_modification.md) | 修饰元素调和的转换结果 |
| [`elemental_tuning_completed`](events/elemental_tuning_completed.md) | 元素调和完成后的通知 |
| [`card_will_be_played`](events/card_will_be_played.md) | 手牌效果生效前的事件 |
| [`card_effect`](events/card_effect.md) | 执行打出的牌自身效果 |
| [`card_played`](events/card_played.md) | 打出手牌完成后的通知 |
| [`skill_will_be_used`](events/skill_will_be_used.md) | 技能效果生效前的事件 |
| [`skill_effect`](events/skill_effect.md) | 执行所选技能自身效果 |
| [`skill_used`](events/skill_used.md) | 技能使用完成后的通知 |
| [`prepared_skill_effect`](events/prepared_skill_effect.md) | 消耗准备技能附属并执行其自身效果 |

## 伤害、治疗与元素

| | |
| --- | --- |
| [`relative_character_target`](events/relative_character_target.md) | 按出战位置的有符号偏移定位角色 |
| [`damage_preparation`](events/damage_preparation.md) | 伤害来源、目标、元素与标志的属性修饰 |
| [`damage_calculation`](events/damage_calculation.md) | 伤害计算事件 |
| [`damage_effect`](events/damage_effect.md) | 扣除生命前的伤害结算事件 |
| [`after_damage`](events/after_damage.md) | 伤害及其元素附着结算完成后的通知 |
| [`healing`](events/healing.md) | 角色恢复生命前的治疗调整事件 |
| [`healed`](events/healed.md) | 角色治疗完成后的通知 |
| [`elemental_reaction_will_occur`](events/elemental_reaction_will_occur.md) | 反应判定后选择替代效果的事件 |
| [`after_elemental_reaction`](events/after_elemental_reaction.md) | 元素反应处理完成后的通知 |

## 角色与实体

| | |
| --- | --- |
| [`active_character_changed`](events/active_character_changed.md) | 出战角色设置完成后的通知 |
| [`character_will_be_defeated`](events/character_will_be_defeated.md) | 角色被击倒前的事件 |
| [`character_defeated`](events/character_defeated.md) | 角色被击倒后的通知 |
| [`support_removed`](events/support_removed.md) | 支援移除后向其他有效实体广播的通知 |
| [`support_state_changed`](events/support_state_changed.md) | 支援状态修改后的自身通知 |
| [`summon_removed`](events/summon_removed.md) | 召唤物移除后向其他有效实体广播的通知 |
| [`combat_status_removed`](events/combat_status_removed.md) | 出战状态移除后向其他有效实体广播的通知 |
| [`attachment_removed`](events/attachment_removed.md) | 角色附属实体或装备移除后向其他有效实体广播的通知 |
| [`resummoning`](events/resummoning.md) | 仅向已有召唤物发送的重复请求 |
| [`summon_state_changed`](events/summon_state_changed.md) | 状态修改后的自身通知 |
| [`combat_status_regeneration`](events/combat_status_regeneration.md) | 仅向已有出战状态发送的重复请求 |
| [`combat_status_state_changed`](events/combat_status_state_changed.md) | 状态修改后的自身通知 |
| [`attachment_reapplication`](events/attachment_reapplication.md) | 仅向已有角色附属实体发送的重复请求 |
| [`attachment_state_changed`](events/attachment_state_changed.md) | 状态修改后的自身通知 |

## 标识类型别名

| | |
| --- | --- |
| [`skill_target_id`](events/skill_target_id.md) | 技能效果的目标标识 |
| [`card_target_id`](events/card_target_id.md) | 卡牌效果的目标标识 |
| [`damage_source_id`](events/damage_source_id.md) | 伤害的来源标识 |
| [`damage_target`](events/damage_target.md) | 动态伤害的具体或相对目标 |
| [`effect_source_id`](events/effect_source_id.md) | 治疗等效果的来源标识 |
| [`element_application_source_id`](events/element_application_source_id.md) | 元素附着的来源标识 |

## 调试

| | |
| --- | --- |
| [`test_event`](events/test_event.md) | 事件响应的测试通知 |

## 特技

| | |
| --- | --- |
| [`cost_of_technique`](events/cost_of_technique.md) | 特技报价 |
| [`technique_will_be_used`](events/technique_will_be_used.md) | 特技使用前广播 |
| [`technique_effect`](events/technique_effect.md) | 特技自身效果 |
| [`technique_used`](events/technique_used.md) | 特技使用后广播 |
| [`technique_target_id`](events/technique_target_id.md) | 特技目标 |
