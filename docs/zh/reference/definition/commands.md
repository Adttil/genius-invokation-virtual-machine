[givm](../../reference.md) / [定义](../definition.md) / **命令**

# 命令

命令描述对局中要完成的操作，例如抽牌、投骰、切换出战角色和造成伤害。把这些操作组合起来，可以决定开局准备、每回合流程，以及实体响应事件时产生的效果。

规则流程与事件效果只能组合核心给定集合中的命令；公开接口不支持自行定义新的命令类型。

命令是否消费响应输入，由编译时给出的具体命令值决定。每个动态命令要求响应通过 `invoke` 提交一个由其成员类型 `input_type` 指定的 [命令输入](command_inputs.md) 对象；使用固定参数的命令不占输入位置。每个已编译入口所需输入对象的数量、类型和顺序固定，对象中的数组长度可以在响应时决定。仅支持固定模式的命令没有 `input_type`。输入类型可以显式复用事件类型，但命令不借用外层响应事件。相对玩家参数以当前效果的本方为基准。命令的内部执行函数不属于公开接口；对局通过 [执行器](../executor/executor.md) 推进，并通过执行现场观察结果和提交行动输入。

`set_active_character{}`、`use_skill{}`、`set_skill_state{}`、`set_energy{}`、`modify_energy{}`、`deal_damage{}`、`apply_element{}`、`heal{}`、`increase_max_health{}`、`create_hand_card{}`、`add_dice{}`，以及实体生成、添加、状态设置、按增量修改和移除命令默认构造时采用动态输入；也可以显式指定固定参数，具体用法见各自页面。按定义定位实体的命令通过显式指定 `definition` 选择固定模式。

固定效果命令使用角色位置、本方或对方身份、定义 ID，不保存对局中才分配的实体 ID。具体实体 ID 由响应在运行时通过动态输入提交。角色位置在命令执行时解析，仅使用相对于所选一方当前出战角色的有符号偏移。

[`relative_player`](commands/relative_player.md) 的 `self`、`opponent` 相对于 [`table_state::self_player`](../table/table_state.md)。响应程序执行期间本方为响应实体所属玩家；嵌套效果结束后恢复外层。根流程默认没有本方，需要使用相对效果命令时须显式设置，不能自动采用当前行动玩家。`insert_deck_card`、`enter_character`、`shuffle_deck`、`replace_cards` 等根流程命令仍使用明确的 `player_id`。

## 开局与牌堆

| | |
| --- | --- |
| [`insert_deck_card`](commands/insert_deck_card.md) | 向牌堆插入指定牌的命令 |
| [`enter_character`](commands/enter_character.md) | 角色入场命令 |
| [`shuffle_deck`](commands/shuffle_deck.md) | 洗牌命令 |
| [`set_active_character`](commands/set_active_character.md) | 按固定目标或响应输入设置出战角色 |
| [`select_active_character_both`](commands/select_active_character_both.md) | 双方开局出战角色的选择命令 |
| [`draw_cards`](commands/draw_cards.md) | 抽牌命令 |
| [`create_hand_card`](commands/create_hand_card.md) | 向手牌中生成一张指定牌的命令 |
| [`discard_hand_card`](commands/discard_hand_card.md) | 舍弃手牌，先处理自身效果再全场通知 |
| [`discard_deck_cards`](commands/discard_deck_cards.md) | 整批舍弃牌堆顶的牌，再逐张处理自身效果和全场通知 |
| [`replace_cards`](commands/replace_cards.md) | 单方换牌命令 |
| [`replace_cards_both`](commands/replace_cards_both.md) | 双方开局换牌命令 |

## 回合推进

| | |
| --- | --- |
| [`start_round`](commands/start_round.md) | 投骰后显式发送回合开始规则通知 |
| [`start_dice_roll_phase`](commands/start_dice_roll_phase.md) | 双方投骰阶段的处理命令 |
| [`start_battle`](commands/start_battle.md) | 首回合战斗开始的通知命令 |
| [`begin_action`](commands/begin_action.md) | 行动阶段的处理命令 |
| [`end_round`](commands/end_round.md) | 回合结束命令 |
| [`end_game`](commands/end_game.md) | 按指定胜负结果结束对局的命令 |

## 技能、伤害与元素

| | |
| --- | --- |
| [`add_dice`](commands/add_dice.md) | 增加指定种类和数量的元素骰并通知的命令 |
| [`use_skill`](commands/use_skill.md) | 直接使用技能并通知，不支付技能费用 |
| [`set_skill_state`](commands/set_skill_state.md) | 技能完整状态的赋值命令 |
| [`set_energy`](commands/set_energy.md) | 角色充能的赋值命令 |
| [`modify_energy`](commands/modify_energy.md) | 按有符号增量修改角色充能的命令 |
| [`deal_damage`](commands/deal_damage.md) | 完成一组可由范围和元素反应展开的伤害 |
| [`heal`](commands/heal.md) | 调整治疗量、恢复生命并通知实际恢复值 |
| [`increase_max_health`](commands/increase_max_health.md) | 增加生命上限，恢复相同数量生命并通知 |
| [`apply_element`](commands/apply_element.md) | 元素附着命令 |

## 支援、召唤物、状态与装备

| | |
| --- | --- |
| [`add_support`](commands/add_support.md) | 向支援区添加独立实体，满区时无效 |
| [`set_support_state`](commands/set_support_state.md) | 设置支援的完整状态 |
| [`modify_support_state`](commands/modify_support_state.md) | 按增量修改支援的层数和次数 |
| [`remove_support`](commands/remove_support.md) | 移除支援并通知离场 |
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
| [`relative_player`](commands/relative_player.md) | 相对于当前效果本方的一方 |
| [`relative_character_target`](events/relative_character_target.md) | 执行时解析的角色位置 |
| [`fixed_damage`](commands/fixed_damage.md) | 固定伤害组中的单段描述 |
| [`character_selection`](commands/character_selection.md) | 选择定位角色、其他角色或全部角色 |
| [`action_argument`](../executor/action_argument.md) | 行动输入参数 |
