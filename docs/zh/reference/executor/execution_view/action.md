[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<action>**

# givm::execution_view<execution_state::action>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::action>;
```

行动选择与费用预览现场的视图，包含当前切换候选的费用及相应输入操作。

## 成员函数

| | |
| --- | --- |
| [`costs`](action/costs.md) | 取得当前切换候选的费用。 |
| [`request_cost`](action/request_cost.md) | 请求计算指定候选的费用。 |
| [`execute_action`](action/execute_action.md) | 提交指定候选的行动，在执行时重新计算费用。 |
| [`execute_action_with_cost`](action/execute_action_with_cost.md) | 提交指定候选的行动，采用该候选已经计算的费用。 |
| [`declare_round_end`](action/declare_round_end.md) | 填写当前玩家宣布结束回合的请求。 |

## 注意

候选下标按当前玩家存活、非出战角色的遍历顺序确定。刚建立现场时费用为默认值；费用预览由 request_cost 请求，下一次推进完成计算后重新取得视图读取。提交已经预览的费用不会重新计算，调用方须保证所选费用可用且支付符合它的要求。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
