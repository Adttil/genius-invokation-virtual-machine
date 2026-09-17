[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<action_selection>**

# givm::execution_view<execution_state::action_selection>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::action_selection>;
```

选择行动的现场视图，用于打出手牌、切换出战角色或宣布结束本回合，并提供选择前所需的费用与合法性查询。

## 成员函数

| | |
| --- | --- |
| [`card_count`](action_selection/card_count.md) | 取得当前出牌候选数量。 |
| [`card_id`](action_selection/card_id.md) | 按出牌候选索引取得手牌 ID。 |
| [`card_cost`](action_selection/card_cost.md) | 取得指定手牌的当前出牌费用。 |
| [`calculate_card_cost`](action_selection/calculate_card_cost.md) | 计算指定手牌的出牌费用并立即返回结果。 |
| [`card_payment_validate`](action_selection/card_payment_validate.md) | 检查出牌骰子的费用匹配与持有数量。 |
| [`card_targets_validate`](action_selection/card_targets_validate.md) | 请牌定义分步检查目标，并告知能否完成或继续选择。 |
| [`play_card`](action_selection/play_card.md) | 选择手牌、支付骰子及至多两个目标。 |
| [`switch_target_count`](action_selection/switch_target_count.md) | 取得当前切换候选数量。 |
| [`switch_target`](action_selection/switch_target.md) | 按切换候选索引取得角色 ID。 |
| [`switch_cost`](action_selection/switch_cost.md) | 取得指定角色的当前切换费用。 |
| [`calculate_switch_cost`](action_selection/calculate_switch_cost.md) | 计算切换至指定角色的费用并立即返回结果。 |
| [`switch_payment_validate`](action_selection/switch_payment_validate.md) | 检查所选骰子是否满足切换费用及持有数量。 |
| [`switch_active_character`](action_selection/switch_active_character.md) | 选择切换角色及支付骰子，可采用已计算费用或同步重新报价。 |
| [`declare_round_end`](action_selection/declare_round_end.md) | 填写当前玩家宣布结束回合的请求。 |

## 注意

在本现场继续调用 [`executor::step`](../executor/step.md) 前，调用方必须通过 [`play_card`](action_selection/play_card.md)、[`switch_active_character`](action_selection/switch_active_character.md) 或 [`declare_round_end`](action_selection/declare_round_end.md) 提供行动输入。费用预览、支付检查与目标检查不算行动输入；尚未提供输入时，上层应保留当前现场，不调用 `step`。

出牌与切换分别使用从零开始的候选索引。可通过 `card_count` 与 `switch_target_count` 查询候选数量，通过 `card_id` 与 `switch_target` 取得相应实体 ID，用于查询牌桌并显示候选信息；候选索引仅用于当前行动现场。

出牌候选只包含当前行动玩家仍在手中的有效手牌。牌的效果目标仍使用 ID，检查与出牌接口接收目标 span，默认空 span 表示不选目标，只采用前两个元素。目标检查可以从空选择开始，并区分无效、必须继续选择、可以完成也可以继续，以及已完成且不能继续。支付与目标检查相互独立，由调用方按需使用，提交时不会自动执行检查。

切换候选只包含当前行动玩家存活、非出战的角色；费用响应不能改变目标。刚建立现场时费用为默认值；费用预览同步更新指定候选的报价。支付检查及采用已计算费用的选择操作要求该候选已经完整报价，由调用方保证；提交行动时不会自动检查支付是否合法。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
