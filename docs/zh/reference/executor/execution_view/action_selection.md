[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<action_selection>**

# givm::execution_view<execution_state::action_selection>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::action_selection>;
```

选择行动的现场视图，用于使用技能、打出手牌、元素调和、切换出战角色或宣布结束本回合，并提供选择前所需的费用与合法性查询。

## 成员函数

| | |
| --- | --- |
| [`skill_count`](action_selection/skill_count.md) | 取得当前技能候选数量。 |
| [`skill_id`](action_selection/skill_id.md) | 按技能候选索引取得技能 ID。 |
| [`skill_cost`](action_selection/skill_cost.md) | 取得指定技能的当前使用费用。 |
| [`calculate_skill_cost`](action_selection/calculate_skill_cost.md) | 计算指定技能的费用并立即返回结果。 |
| [`skill_payment_validate`](action_selection/skill_payment_validate.md) | 检查支付骰子及出战角色充能。 |
| [`skill_targets_validate`](action_selection/skill_targets_validate.md) | 请技能定义分步检查目标及使用条件。 |
| [`use_skill`](action_selection/use_skill.md) | 选择技能、支付骰子及至多两个目标。 |
| [`card_count`](action_selection/card_count.md) | 取得当前手牌候选数量。 |
| [`card_id`](action_selection/card_id.md) | 按手牌候选索引取得手牌 ID。 |
| [`card_cost`](action_selection/card_cost.md) | 取得指定手牌的当前出牌费用。 |
| [`calculate_card_cost`](action_selection/calculate_card_cost.md) | 计算指定手牌的出牌费用并立即返回结果。 |
| [`card_payment_validate`](action_selection/card_payment_validate.md) | 检查支付骰子的费用匹配、持有数量及出战角色充能。 |
| [`card_targets_validate`](action_selection/card_targets_validate.md) | 请牌定义分步检查目标，并告知能否完成或继续选择。 |
| [`play_card`](action_selection/play_card.md) | 选择手牌、支付骰子及至多两个目标。 |
| [`elemental_tuning_card_validate`](action_selection/elemental_tuning_card_validate.md) | 检查手牌是否允许元素调和。 |
| [`elemental_tuning_dice_validate`](action_selection/elemental_tuning_dice_validate.md) | 检查选中的骰子能否用于元素调和。 |
| [`elemental_tuning`](action_selection/elemental_tuning.md) | 选择手牌与一枚元素骰进行调和。 |
| [`switch_target_count`](action_selection/switch_target_count.md) | 取得当前切换候选数量。 |
| [`switch_target`](action_selection/switch_target.md) | 按切换候选索引取得角色 ID。 |
| [`switch_cost`](action_selection/switch_cost.md) | 取得指定角色的当前切换费用。 |
| [`calculate_switch_cost`](action_selection/calculate_switch_cost.md) | 计算切换至指定角色的费用并立即返回结果。 |
| [`switch_payment_validate`](action_selection/switch_payment_validate.md) | 检查支付骰子的费用匹配、持有数量及出战角色充能。 |
| [`switch_active_character`](action_selection/switch_active_character.md) | 选择切换角色及支付骰子，可采用已计算费用或同步计算报价。 |
| [`declare_round_end`](action_selection/declare_round_end.md) | 填写当前玩家宣布结束回合的请求。 |

## 注意

在本现场继续调用 [`executor::step`](../executor/step.md) 前，调用方必须通过 [`use_skill`](action_selection/use_skill.md)、[`play_card`](action_selection/play_card.md)、[`elemental_tuning`](action_selection/elemental_tuning.md)、[`switch_active_character`](action_selection/switch_active_character.md) 或 [`declare_round_end`](action_selection/declare_round_end.md) 提供行动输入。费用预览、支付检查与目标检查不算行动输入；尚未提供输入时，上层应保留当前现场，不调用 `step`。

技能、出牌与切换分别使用从零开始的候选索引。可通过 `skill_count`、`card_count` 与 `switch_target_count` 查询候选数量，通过 `skill_id`、`card_id` 与 `switch_target` 取得相应实体 ID，用于查询牌桌并显示候选信息；候选索引仅用于当前行动现场。

技能候选只包含当前行动玩家出战角色中支持 [`skill_effect`](../../definition/events/skill_effect.md) 响应的有效技能。不支持主动效果的被动技能仍保留在角色技能集合中，但不作为主动行动候选。技能目标同样使用 ID 和至多两个元素的目标 span，支付与目标检查彼此独立。

出牌与调和共享当前行动玩家仍在手中的有效手牌候选。调和无需计算费用，卡牌许可与骰子合法性可以分别检查；完成后仍由当前玩家选择行动。

牌的效果目标仍使用 ID，检查与出牌接口接收目标 span，默认空 span 表示不选目标，只采用前两个元素。目标检查可以从空选择开始，并区分无效、必须继续选择、可以完成也可以继续，以及已完成且不能继续。支付与目标检查相互独立，由调用方按需使用，提交时不会自动执行检查。

切换候选只包含当前行动玩家存活、非出战的角色；费用响应不能改变目标。费用预览同步计算指定候选的报价。每个候选在当前行动窗口只能计算一次，之后可反复读取；调用方自行保证，库不进行运行期检查。支付检查及采用已计算费用的选择操作要求该候选已经完整报价，由调用方保证；提交行动时不会自动检查支付是否合法。

充能属于统一费用，直接从支付阶段的出战角色扣除；行动输入只选择骰子，不单独选择充能。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
