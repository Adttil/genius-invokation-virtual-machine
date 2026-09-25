[givm](../../reference.md) / [定义](../definition.md) / **命令输入**

# 命令输入

命令输入描述一次响应产生的具体效果参数。支持动态输入的命令通过成员类型 `input_type` 指定对应的 `xxx_input`；每条动态命令消费一个此类型的对象，固定模式不占输入位置。仅支持固定模式的命令没有 `input_type`。响应按命令执行顺序通过 [`handle_context::invoke`](../executor/handle_context/invoke.md) 提交。

输入对象的数量、类型和顺序由已登记的程序决定；对象中的数组长度可以运行时确定。数组内容在 `invoke` 时复制，提交后不再借用原数组。未定义 `NDEBUG` 时检查对象数量、具体类型及顺序；发布构建不检查，错误输入属于未定义行为。

输入类型与命令一同定义于 `<givm/definition/commands.hpp>`，紧邻相应命令。字段已由某个 [事件](events.md) 完整表达时，`xxx_input` 可直接是该事件的别名；是否触发广播由命令决定。其他输入使用独立结构体。

## 类

| | |
| --- | --- |
| [`add_attachment_input`](command_inputs/add_attachment_input.md) | [`add_attachment`](commands/add_attachment.md) 的动态输入 |
| [`add_combat_status_input`](command_inputs/add_combat_status_input.md) | [`add_combat_status`](commands/add_combat_status.md) 的动态输入 |
| [`add_summon_input`](command_inputs/add_summon_input.md) | [`add_summon`](commands/add_summon.md) 的动态输入 |
| [`add_support_input`](command_inputs/add_support_input.md) | [`add_support`](commands/add_support.md) 的动态输入 |
| [`apply_element_input`](command_inputs/apply_element_input.md) | [`apply_element`](commands/apply_element.md) 的动态输入 |
| [`attach_input`](command_inputs/attach_input.md) | [`attach`](commands/attach.md) 的动态输入 |
| [`create_hand_card_input`](command_inputs/create_hand_card_input.md) | [`create_hand_card`](commands/create_hand_card.md) 的动态输入 |
| [`deal_damage_input`](command_inputs/deal_damage_input.md) | [`deal_damage`](commands/deal_damage.md) 的动态输入 |
| [`discard_deck_cards_input`](command_inputs/discard_deck_cards_input.md) | [`discard_deck_cards`](commands/discard_deck_cards.md) 的动态输入 |
| [`generate_combat_status_input`](command_inputs/generate_combat_status_input.md) | [`generate_combat_status`](commands/generate_combat_status.md) 的动态输入 |
| [`heal_input`](command_inputs/heal_input.md) | [`heal`](commands/heal.md) 的动态输入 |
| [`modify_attachment_state_input`](command_inputs/modify_attachment_state_input.md) | [`modify_attachment_state`](commands/modify_attachment_state.md) 的动态输入 |
| [`modify_combat_status_state_input`](command_inputs/modify_combat_status_state_input.md) | [`modify_combat_status_state`](commands/modify_combat_status_state.md) 的动态输入 |
| [`modify_energy_input`](command_inputs/modify_energy_input.md) | [`modify_energy`](commands/modify_energy.md) 的动态输入 |
| [`modify_summon_state_input`](command_inputs/modify_summon_state_input.md) | [`modify_summon_state`](commands/modify_summon_state.md) 的动态输入 |
| [`modify_support_state_input`](command_inputs/modify_support_state_input.md) | [`modify_support_state`](commands/modify_support_state.md) 的动态输入 |
| [`remove_attachment_input`](command_inputs/remove_attachment_input.md) | [`remove_attachment`](commands/remove_attachment.md) 的动态输入 |
| [`remove_combat_status_input`](command_inputs/remove_combat_status_input.md) | [`remove_combat_status`](commands/remove_combat_status.md) 的动态输入 |
| [`remove_summon_input`](command_inputs/remove_summon_input.md) | [`remove_summon`](commands/remove_summon.md) 的动态输入 |
| [`remove_support_input`](command_inputs/remove_support_input.md) | [`remove_support`](commands/remove_support.md) 的动态输入 |
| [`set_attachment_state_input`](command_inputs/set_attachment_state_input.md) | [`set_attachment_state`](commands/set_attachment_state.md) 的动态输入 |
| [`set_combat_status_state_input`](command_inputs/set_combat_status_state_input.md) | [`set_combat_status_state`](commands/set_combat_status_state.md) 的动态输入 |
| [`set_energy_input`](command_inputs/set_energy_input.md) | [`set_energy`](commands/set_energy.md) 的动态输入 |
| [`set_skill_state_input`](command_inputs/set_skill_state_input.md) | [`set_skill_state`](commands/set_skill_state.md) 的动态输入 |
| [`set_summon_state_input`](command_inputs/set_summon_state_input.md) | [`set_summon_state`](commands/set_summon_state.md) 的动态输入 |
| [`set_support_state_input`](command_inputs/set_support_state_input.md) | [`set_support_state`](commands/set_support_state.md) 的动态输入 |
| [`summon_input`](command_inputs/summon_input.md) | [`summon`](commands/summon.md) 的动态输入 |
| [`damage`](command_inputs/damage.md) | 动态伤害组中的单段初始描述 |

## 类型别名

| | |
| --- | --- |
| [`add_dice_input`](command_inputs/add_dice_input.md) | [`add_dice`](commands/add_dice.md) 的动态输入 |
| [`discard_hand_card_input`](command_inputs/discard_hand_card_input.md) | [`discard_hand_card`](commands/discard_hand_card.md) 的动态输入 |
| [`increase_max_health_input`](command_inputs/increase_max_health_input.md) | [`increase_max_health`](commands/increase_max_health.md) 的动态输入 |
| [`set_active_character_input`](command_inputs/set_active_character_input.md) | [`set_active_character`](commands/set_active_character.md) 的动态输入 |
| [`use_skill_input`](command_inputs/use_skill_input.md) | [`use_skill`](commands/use_skill.md) 的动态输入 |
| [`any_command_input`](any_command_input.md) | 核心命令输入 variant |
