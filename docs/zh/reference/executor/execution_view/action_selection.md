[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<action_selection>**

# givm::execution_view<execution_state::action_selection>

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<>
class execution_view<execution_state::action_selection>;
```

选择行动的现场视图，用于使用技能、特技、打出手牌、元素调和、切换出战角色或宣布结束本回合，并提供选择前所需的费用与合法性查询。

## 成员函数

| | |
| --- | --- |
| [`is_controlled`](action_selection/is_controlled.md) | 检查当前出战角色是否受控。 |
| [`skill_count`](action_selection/skill_count.md) | 取得当前技能候选数量。 |
| [`skill_id`](action_selection/skill_id.md) | 按技能候选索引取得技能 ID。 |
| [`skill_cost`](action_selection/skill_cost.md) | 取得指定技能的当前使用费用。 |
| [`calculate_skill_cost`](action_selection/calculate_skill_cost.md) | 计算指定技能的费用并立即返回结果。 |
| [`skill_payment_validate`](action_selection/skill_payment_validate.md) | 检查支付骰子及出战角色充能。 |
| [`skill_targets_validate`](action_selection/skill_targets_validate.md) | 请技能定义分步检查目标及使用条件。 |
| [`use_skill`](action_selection/use_skill.md) | 选择技能、支付骰子及至多两个目标。 |
| [`use_skill_with_cached_cost`](action_selection/use_skill_with_cached_cost.md) | 使用已有报价提交并推进行动。 |
| [`has_technique`](action_selection/has_technique.md) | 是否存在主动特技 |
| [`technique_id`](action_selection/technique_id.md) | 取得特技附件 ID |
| [`technique_cost`](action_selection/technique_cost.md) | 读取特技报价 |
| [`calculate_technique_cost`](action_selection/calculate_technique_cost.md) | 计算特技报价 |
| [`technique_payment_validate`](action_selection/technique_payment_validate.md) | 检查特技支付 |
| [`technique_targets_validate`](action_selection/technique_targets_validate.md) | 分步检查特技目标 |
| [`use_technique`](action_selection/use_technique.md) | 选择使用特技 |
| [`use_technique_with_cached_cost`](action_selection/use_technique_with_cached_cost.md) | 使用已有报价提交并推进行动。 |
| [`card_count`](action_selection/card_count.md) | 取得当前手牌候选数量。 |
| [`card_id`](action_selection/card_id.md) | 按手牌候选索引取得手牌 ID。 |
| [`card_cost`](action_selection/card_cost.md) | 取得指定手牌的当前出牌费用。 |
| [`calculate_card_cost`](action_selection/calculate_card_cost.md) | 计算指定手牌的出牌费用并立即返回结果。 |
| [`card_payment_validate`](action_selection/card_payment_validate.md) | 检查支付骰子的费用匹配、持有数量及出战角色充能。 |
| [`card_targets_validate`](action_selection/card_targets_validate.md) | 请牌定义分步检查目标，并告知能否完成或继续选择。 |
| [`play_card`](action_selection/play_card.md) | 选择手牌、支付骰子及至多两个目标。 |
| [`play_card_with_cached_cost`](action_selection/play_card_with_cached_cost.md) | 使用已有报价提交并推进行动。 |
| [`elemental_tuning_card_validate`](action_selection/elemental_tuning_card_validate.md) | 检查手牌是否允许元素调和。 |
| [`elemental_tuning_dice_validate`](action_selection/elemental_tuning_dice_validate.md) | 检查选中的骰子能否用于元素调和。 |
| [`elemental_tuning`](action_selection/elemental_tuning.md) | 选择手牌与一枚元素骰进行调和。 |
| [`switch_target_count`](action_selection/switch_target_count.md) | 取得当前切换候选数量。 |
| [`switch_target`](action_selection/switch_target.md) | 按切换候选索引取得角色 ID。 |
| [`switch_cost`](action_selection/switch_cost.md) | 取得指定角色的当前切换费用。 |
| [`calculate_switch_cost`](action_selection/calculate_switch_cost.md) | 计算切换至指定角色的费用并立即返回结果。 |
| [`switch_payment_validate`](action_selection/switch_payment_validate.md) | 检查支付骰子的费用匹配、持有数量及出战角色充能。 |
| [`switch_active_character`](action_selection/switch_active_character.md) | 选择切换角色及支付骰子，可采用已计算费用或同步计算报价。 |
| [`switch_active_character_with_cached_cost`](action_selection/switch_active_character_with_cached_cost.md) | 使用已有报价提交并推进行动。 |
| [`declare_round_end`](action_selection/declare_round_end.md) | 提交当前玩家的结束回合声明并推进。 |

## 注意

执行器在提供本现场之前已经检查准备技能。未受控制的出战角色若存在支持 [`this_prepared_skill_use`](../../definition/events/this_prepared_skill_use.md) 的附属，会先自动执行该行动，不提供本次选择现场；受控制时保留准备技能附属并正常提供选择。

本现场通过 `use_technique`、`use_skill`、`play_card`、`elemental_tuning`、`switch_active_character` 或 `declare_round_end` 提交并推进行动；费用行动也可使用相应的 `_with_cached_cost` 版本。费用预览、支付检查与目标检查不算行动输入，不推进执行器。本现场不提供 `resume`。

技能、出牌与切换分别使用从零开始的候选索引。可通过 `skill_count`、`card_count` 与 `switch_target_count` 查询候选数量，通过 `skill_id`、`card_id` 与 `switch_target` 取得相应实体 ID，用于查询牌桌并显示候选信息；候选索引仅用于当前行动现场。

技能候选只包含当前行动玩家出战角色中支持 [`this_skill_use`](../../definition/events/this_skill_use.md) 响应的有效技能。不支持主动效果的被动技能仍保留在角色技能集合中，但不作为主动行动候选。技能目标同样使用 ID 和至多两个元素的目标 span，支付与目标检查彼此独立。

出牌与调和共享当前行动玩家仍在手中的有效手牌候选。调和无需计算费用，卡牌许可与骰子合法性可以分别检查；完成后仍由当前玩家选择行动。

牌的效果目标仍使用 ID，检查与出牌接口接收目标 span，默认空 span 表示不选目标，只采用前两个元素。目标检查可以从空选择开始，并区分无效、必须继续选择、可以完成也可以继续，以及已完成且不能继续。支付与目标检查相互独立，由调用方按需使用，Debug 报价时自动逐步检查，Release 不执行检查。

费用报价绑定具体操作和完整目标。每次报价返回强类型标识，用它读取报价、检查支付或采用缓存；同一候选的不同目标可以分别报价，核心只为实际查询的组合准备缓存。每个窗口内同一操作和目标组合只报价一次，Debug 检查重复报价，Release 由调用方保证。

采用缓存的接口只接收报价标识和支付骰子，不重新指定来源或目标。直接行动接口则完成报价后立即执行。报价、读取费用及目标检查均不推进执行器；费用响应不能修改牌桌或使用随机数。

确认行动时锁定原选骰子、原付费角色以及重击等行动标记。各费用程序分别完整结算后，逐色尽可能扣除原选数量，其他颜色或万能骰不补足；充能从原付费角色饱和扣除。通知报告实际变化，并归入实际行动段。费用效果终局后不再付款或执行行动。

报价标识在追加其他报价和复制既有行动窗口后仍可定位对应结果；窗口结束后失效。具体费用引用可能因报价扩容失效，需要通过标识重新取得。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |

特技是 `equipment_type::technique` 装备附件，至多一个，接口无需候选索引。它与技能独立，费用和目标查询由附件定义提供；受控时技能和特技都不能使用。

## 普通攻击的特殊性质

技能定义带 `normal_attack` 标签时，建立行动候选会按支付前的骰子总数判定重击：偶数（包括零）具有 `charged_attack`。若该玩家仍有 `can_plunge` 资格，同时具有 `plunging_attack`。两者可以共存，均出现在 `cost_of_skill::flags` 中，因此报价可先按重击减费，再支付折扣后的费用。支付不会重新判定这些性质。

只有行动选择中的普通攻击候选自动获得这些性质；特技、手牌以及其他效果产生的伤害不会因骰子数量自动获得重击性质。定义需要将技能效果事件的 flags 显式转换并填写进其伤害。
