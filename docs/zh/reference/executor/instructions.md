[givm](../../reference.md) / [执行](../executor.md) / **指令**

# 指令

指令描述对局中要完成的操作，例如抽牌、投骰、切换出战角色和造成伤害。把这些操作组合起来，可以决定开局准备、每回合流程，以及实体响应事件时产生的效果。

规则流程与事件效果只能组合核心给定集合中的指令；公开接口不支持自行定义新的指令类型。

具有 `context_type = void` 的指令可用于一般操作；具有事件类型的指令只能放入对应事件的响应。例如 [`absorb_damage_by_count`](instructions/absorb_damage_by_count.md) 用于伤害结算时的抵挡效果。指令用于描述规则，其内部执行函数不属于公开接口；对局通过 [`executor`](executor.md) 推进，并通过执行现场观察结果和提交输入。

## 开局与牌堆

| | |
| --- | --- |
| [`insert_deck_card`](instructions/insert_deck_card.md) | 向牌堆插入指定牌的指令 |
| [`enter_character`](instructions/enter_character.md) | 角色入场指令 |
| [`initialize_characters`](instructions/initialize_characters.md) | 已有角色的初始化指令 |
| [`shuffle_deck`](instructions/shuffle_deck.md) | 洗牌指令 |
| [`set_active_character`](instructions/set_active_character.md) | 直接设置出战角色的指令 |
| [`select_active_character_both`](instructions/select_active_character_both.md) | 双方开局出战角色的选择指令 |
| [`draw_cards`](instructions/draw_cards.md) | 抽牌指令 |
| [`replace_cards`](instructions/replace_cards.md) | 单方换牌指令 |
| [`replace_cards_both`](instructions/replace_cards_both.md) | 双方开局换牌指令 |

## 回合推进

| | |
| --- | --- |
| [`start_round`](instructions/start_round.md) | 新回合的开始指令 |
| [`start_dice_roll_phase`](instructions/start_dice_roll_phase.md) | 双方投骰阶段的处理指令 |
| [`start_battle`](instructions/start_battle.md) | 首回合战斗开始的通知指令 |
| [`begin_action`](instructions/begin_action.md) | 行动阶段的处理指令 |
| [`end_round`](instructions/end_round.md) | 回合结束指令 |
| [`end_game`](instructions/end_game.md) | 按指定胜负结果结束对局的指令 |

## 伤害与元素

| | |
| --- | --- |
| [`deal_damage`](instructions/deal_damage.md) | 伤害结算指令 |
| [`absorb_damage_by_count`](instructions/absorb_damage_by_count.md) | 消耗实体计数以抵挡伤害的指令 |
| [`apply_element`](instructions/apply_element.md) | 元素附着指令 |
| [`set_element_aura`](instructions/set_element_aura.md) | 直接设置元素附着的指令 |

## 调试

| | |
| --- | --- |
| [`test_command`](instructions/test_command.md) | 事件响应的测试指令 |

## 选择与行动枚举

| | |
| --- | --- |
| [`relative_player`](instructions/relative_player.md) | 相对于当前行动玩家的一方 |
| [`action_target_kind`](instructions/action_target_kind.md) | 行动目标的种类 |

## 行动参数

| | |
| --- | --- |
| [`action_target`](instructions/action_target.md) | 行动的目标 |
| [`action_argument`](instructions/action_argument.md) | 执行行动时提交的支付骰子与目标 |
