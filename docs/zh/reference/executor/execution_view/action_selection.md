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
| [`costs`](action_selection/costs.md) | 取得当前切换候选的费用。 |
| [`calculate_cost`](action_selection/calculate_cost.md) | 计算指定候选的费用并立即返回结果。 |
| [`check_payment`](action_selection/check_payment.md) | 检查所选骰子是否满足候选费用及持有数量。 |
| [`execute_action`](action_selection/execute_action.md) | 提交指定候选的行动，在执行时重新计算费用。 |
| [`execute_action_with_cost`](action_selection/execute_action_with_cost.md) | 提交指定候选的行动，采用该候选已经计算的费用。 |
| [`declare_round_end`](action_selection/declare_round_end.md) | 填写当前玩家宣布结束回合的请求。 |

## 注意

候选下标按当前玩家存活、非出战角色的遍历顺序确定，费用响应不能改变候选目标。刚建立现场时费用为默认值；费用预览同步更新指定候选。支付检查要求该候选已经完整计算费用，提交行动时不会自动检查支付是否合法。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
