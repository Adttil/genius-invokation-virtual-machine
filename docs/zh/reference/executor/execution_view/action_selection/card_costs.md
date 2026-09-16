[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **card_costs**

# givm::execution_view<execution_state::action_selection>::card_costs

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr std::span<const cost_of_card> card_costs() const noexcept;
```
[`cost_of_card`](../../../definition/events/cost_of_card.md)

取得当前行动玩家手牌的出牌费用。

## 返回值

借用当前现场的只读费用范围，每项的 `card` 指明对应的手牌。每张牌的费用须通过 [`calculate_card_cost`](calculate_card_cost.md) 完整计算后，才能检查支付或采用。

## 注意

本操作只读取费用，不计算费用、不选择出牌。重新计算某张牌后，对应结果随之更新；范围不保存独立快照。下一次推进或重建现场后，先前取得的范围失效。

报价抛出异常后，对应费用可能只更新了一部分；须完整重算成功后才能检查或采用。

