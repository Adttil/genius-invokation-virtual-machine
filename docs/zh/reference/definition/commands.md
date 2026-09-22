[givm](../../reference.md) / [定义](../definition.md) / **命令**

# 命令

命令描述对局中要完成的操作，例如抽牌、投骰、切换出战角色和造成伤害。把这些操作组合起来，可以决定开局准备、每回合流程，以及实体响应事件时产生的效果。

规则流程与事件效果只能组合核心给定集合中的命令；公开接口不支持自行定义新的命令类型。

命令是否消费响应输入，由编译时给出的具体命令值决定。消费输入的命令要求响应通过 `invoke` 显式提交相应初始事件；使用固定参数的命令不占输入位置。每个已编译入口所需输入的数量、类型和顺序仍然固定，命令不借用外层响应事件或响应者。命令的内部执行函数不属于公开接口；对局通过 [执行器](../executor/executor.md) 推进，并通过执行现场观察结果和提交行动输入。

`set_active_character{}`、`deal_damage{}`，以及实体生成、添加、状态设置、按增量修改和移除命令默认构造时采用动态输入；也可以显式指定固定参数，具体用法见各自页面。实体相关命令通过显式指定 `definition` 选择固定模式。

## 开局与牌堆

| | |
| --- | --- |
| [`insert_deck_card`](commands/insert_deck_card.md) | 向牌堆插入指定牌的命令 |
| [`enter_character`](commands/enter_character.md) | 角色入场命令 |
| [`shuffle_deck`](commands/shuffle_deck.md) | 洗牌命令 |
| [`set_active_character`](commands/set_active_character.md) | 按固定目标或响应输入设置出战角色 |
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
| [`deal_damage`](commands/deal_damage.md) | 完成一组可由范围和元素反应展开的伤害 |
| [`apply_element`](commands/apply_element.md) | 元素附着命令 |
| [`set_element_aura`](commands/set_element_aura.md) | 直接设置元素附着的命令 |

## 召唤物、状态与装备

| | |
| --- | --- |
| [`summon`](commands/summon.md) | 召唤指定实体，由已有同类实体处理重复请求 |
| [`add_summon`](commands/add_summon.md) | 直接添加独立召唤物 |
| [`set_summon_state`](commands/set_summon_state.md) | 设置召唤物的完整状态 |
| [`modify_summon_state`](commands/modify_summon_state.md) | 按增量修改召唤物的状态 |
| [`remove_summon`](commands/remove_summon.md) | 移除召唤物并通知离场 |
| [`generate_combat_status`](commands/generate_combat_status.md) | 生成出战状态，由已有同类实体处理重复请求 |
| [`add_combat_status`](commands/add_combat_status.md) | 直接添加独立出战状态 |
| [`set_combat_status_state`](commands/set_combat_status_state.md) | 设置出战状态的完整状态 |
| [`modify_combat_status_state`](commands/modify_combat_status_state.md) | 按增量修改出战状态的层数和次数 |
| [`remove_combat_status`](commands/remove_combat_status.md) | 移除出战状态并通知离场 |
| [`attach`](commands/attach.md) | 向角色附属状态或装备，由已有同类实体处理重复请求 |
| [`add_attachment`](commands/add_attachment.md) | 直接添加独立角色附属实体 |
| [`set_attachment_state`](commands/set_attachment_state.md) | 设置角色附属实体的完整状态 |
| [`modify_attachment_state`](commands/modify_attachment_state.md) | 按增量修改角色附属实体的状态 |
| [`remove_attachment`](commands/remove_attachment.md) | 移除角色附属实体并通知离场 |

## 调试

| | |
| --- | --- |
| [`test_command`](commands/test_command.md) | 事件响应的测试命令 |

## 参阅

| | |
| --- | --- |
| [`any_command`](any_command.md) | 命令 variant |
| [`relative_player`](commands/relative_player.md) | 相对于当前行动玩家的一方 |
| [`action_argument`](../executor/action_argument.md) | 行动输入参数 |
