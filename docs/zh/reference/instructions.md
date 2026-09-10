[givm](../reference.md) / **指令**

# 指令

指令描述对局中要完成的操作，例如抽牌、投骰、切换出战角色和造成伤害。把这些操作组合起来，可以决定开局准备、每回合流程，以及实体响应事件时产生的效果。

具有 `context_type = void` 的指令可用于一般操作；具有事件类型的指令只能放入对应事件的响应。例如 [`absorb_damage_by_count`](instructions/absorb_damage_by_count.md) 用于伤害结算时的抵挡效果。一次调用总会完整执行一条指令；需要等待玩家输入或其他事件响应的结算，可以在后续调用中重新进入同一条指令。

## 开局与牌堆

| | |
| --- | --- |
| [`insert_deck_card`](instructions/insert_deck_card.md) | 向指定玩家的牌堆插入一张牌 |
| [`enter_character`](instructions/enter_character.md) | 使一个角色加入指定玩家的队伍，并按角色定义准备其初始状态 |
| [`initialize_characters`](instructions/initialize_characters.md) | 按角色定义重新准备指定玩家所有角色的初始状态 |
| [`shuffle_deck`](instructions/shuffle_deck.md) | 随机重排指定玩家牌堆中的牌 |
| [`set_active_character`](instructions/set_active_character.md) | 直接设置一名玩家的出战角色 |
| [`select_active_character_both`](instructions/select_active_character_both.md) | 让双方选择开局出战角色 |
| [`draw_cards`](instructions/draw_cards.md) | 让一名玩家从牌堆顶抽取指定数量的牌 |
| [`replace_cards`](instructions/replace_cards.md) | 让指定玩家选择需要替换的手牌，将所选牌放回牌堆并抽取等量新牌 |
| [`replace_cards_both`](instructions/replace_cards_both.md) | 让双方分别选择开局需要替换的手牌 |

## 回合推进

| | |
| --- | --- |
| [`start_round`](instructions/start_round.md) | 开始下一回合，更新回合数并清空双方的元素骰 |
| [`start_dice_roll_phase`](instructions/start_dice_roll_phase.md) | 为双方开始投骰阶段，先处理固定骰子和重投次数，再生成双方骰子并接受重投选择 |
| [`start_battle`](instructions/start_battle.md) | 在第一回合发出战斗开始通知，让相关效果在双方进入战斗时生效 |
| [`begin_action`](instructions/begin_action.md) | 开始本回合的行动阶段，让玩家依次选择行动，直到双方都宣布结束 |
| [`end_round`](instructions/end_round.md) | 关闭本回合并准备下一回合的先手 |

## 伤害与元素

| | |
| --- | --- |
| [`deal_damage`](instructions/deal_damage.md) | 对一个角色造成伤害 |
| [`absorb_damage_by_count`](instructions/absorb_damage_by_count.md) | 用当前响应实体的次数或层数抵挡伤害 |
| [`apply_element`](instructions/apply_element.md) | 向一个角色施加元素附着 |
| [`set_element_aura`](instructions/set_element_aura.md) | 直接指定角色身上的元素附着 |

## 调试

| | |
| --- | --- |
| [`test_command`](instructions/test_command.md) | 发出测试事件，用来检查实体定义中的事件响应是否能被调用 |

## 选择与行动参数

| | |
| --- | --- |
| [`relative_player`](instructions/relative_player.md) | 以当前行动玩家为基准选择一方。 |
| [`action_kind`](instructions/action_kind.md) | 玩家在行动阶段可以提交的行动种类。 |
| [`action_request_kind`](instructions/action_request_kind.md) | 调用方希望执行器怎样处理一次行动选择。 |
| [`action_target_kind`](instructions/action_target_kind.md) | 行动参数中的目标种类，决定读取哪一个目标标识。 |
| [`action_request`](instructions/action_request.md) | 调用方提交给行动阶段的一次请求。它说明要选择哪种行动，以及是否先计算费用。 |
| [`action_target`](instructions/action_target.md) | 行动所指定的目标。使用 kind 指出目标种类，再填写对应的角色、支援或召唤物标识。 |
| [`action_argument`](instructions/action_argument.md) | 执行一次行动时由调用方提交的支付骰子与目标。 |
