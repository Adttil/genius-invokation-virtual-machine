[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<action_selection>**

# givm::execution_view<execution_state::action_selection>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::action_selection>;
```

选择行动的现场视图，提供当前切换候选的费用、支付检查及相应输入操作。

## 成员函数

| | |
| --- | --- |
| [`switch_costs`](action_selection/switch_costs.md) | 取得当前可切换角色的费用。 |
| [`calculate_switch_cost`](action_selection/calculate_switch_cost.md) | 计算切换至指定角色的费用并立即返回结果。 |
| [`check_switch_payment`](action_selection/check_switch_payment.md) | 检查所选骰子是否满足切换费用及持有数量。 |
| [`switch_active_character`](action_selection/switch_active_character.md) | 选择切换角色及支付骰子，可采用已计算费用或同步重新报价。 |
| [`declare_round_end`](action_selection/declare_round_end.md) | 填写当前玩家宣布结束回合的请求。 |

## 注意

在本现场继续调用 [`executor::step`](../executor/step.md) 前，调用方必须通过 [`switch_active_character`](action_selection/switch_active_character.md) 或 [`declare_round_end`](action_selection/declare_round_end.md) 提供行动输入。费用预览与支付检查不算行动输入；尚未提供输入时，上层应保留当前现场，不调用 `step`。

切换目标使用角色 ID，必须是当前行动玩家存活、非出战的角色；费用响应不能改变目标。刚建立现场时费用为默认值；费用预览同步更新指定角色的报价。支付检查及采用已计算费用的选择操作要求该角色已经完整报价，由调用方保证；提交行动时不会自动检查支付是否合法。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
