[givm](../../reference.md) / [定义](../definition.md) / **命令**

# 命令

命令描述对局中要完成的操作，例如抽牌、投骰、切换出战角色和造成伤害。把这些操作组合起来，可以决定开局准备、每回合流程，以及实体响应事件时产生的效果。

规则流程与事件效果只能组合核心给定集合中的命令；公开接口不支持自行定义新的命令类型。

具有 `context_type = void` 的命令可用于一般操作；具有事件类型的命令只能放入对应事件的响应。例如 [`absorb_damage_by_count`](commands/absorb_damage_by_count.md) 用于伤害结算时的抵挡效果。命令用于描述规则，其内部执行函数不属于公开接口；对局通过 [`executor`](../executor/executor.md) 推进，并通过执行现场观察结果和提交输入。

## 开局与牌堆

| | |
| --- | --- |
| [`insert_deck_card`](commands/insert_deck_card.md) | 向牌堆插入指定牌的命令 |
| [`enter_character`](commands/enter_character.md) | 角色入场命令 |
| [`shuffle_deck`](commands/shuffle_deck.md) | 洗牌命令 |
| [`set_active_character`](commands/set_active_character.md) | 直接设置出战角色的命令 |
| [`select_active_character_both`](commands/select_active_character_both.md) | 双方开局出战角色的选择命令 |
| [`draw_cards`](commands/draw_cards.md) | 抽牌命令 |
| [`replace_cards`](commands/replace_cards.md) | 单方换牌命令 |
| [`replace_cards_both`](commands/replace_cards_both.md) | 双方开局换牌命令 |

## 回合推进

| | |
| --- | --- |
| [`start_round`](commands/start_round.md) | 新回合的开始命令 |
| [`start_dice_roll_phase`](commands/start_dice_roll_phase.md) | 双方投骰阶段的处理命令 |
| [`start_battle`](commands/start_battle.md) | 首回合战斗开始的通知命令 |
| [`begin_action`](commands/begin_action.md) | 行动阶段的处理命令 |
| [`end_round`](commands/end_round.md) | 回合结束命令 |
| [`end_game`](commands/end_game.md) | 按指定胜负结果结束对局的命令 |

## 伤害与元素

| | |
| --- | --- |
| [`deal_damage`](commands/deal_damage.md) | 伤害结算命令 |
| [`absorb_damage_by_count`](commands/absorb_damage_by_count.md) | 消耗实体计数以抵挡伤害的命令 |
| [`apply_element`](commands/apply_element.md) | 元素附着命令 |
| [`set_element_aura`](commands/set_element_aura.md) | 直接设置元素附着的命令 |

## 调试

| | |
| --- | --- |
| [`test_command`](commands/test_command.md) | 事件响应的测试命令 |

## 参阅

| | |
| --- | --- |
| [`any_command_for`](any_command_for.md) | 命令 variant |
| [`command_compatible_with`](command_compatible_with.md) | context 兼容性 |
| [`relative_player`](commands/relative_player.md) | 相对于当前行动玩家的一方 |
| [`action_argument`](../executor/action_argument.md) | 行动输入参数 |
