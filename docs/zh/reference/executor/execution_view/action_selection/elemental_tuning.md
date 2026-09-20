[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **elemental_tuning**

# givm::execution_view<execution_state::action_selection>::elemental_tuning

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void elemental_tuning(std::size_t card_index, elemental_dice from) const noexcept;
```

选择一张手牌与一枚元素骰进行元素调和。下一次推进时，手牌离场，骰子转换为当前出战角色的元素；转换结果可以被调和修饰事件改变。

## 参数

| | |
| --- | --- |
| `card_index` | 当前手牌候选索引，须小于 [`card_count()`](card_count.md)。 |
| `from` | 选择转换的骰子元素。 |

## 返回值

（无）

## 注意

本操作只提交选择，不修改牌桌或推进执行器，也不自动检查输入。调用方须保证该牌允许调和、选择的是持有的基础元素骰，且与出战角色自身元素不同；可分别使用 [`elemental_tuning_card_validate`](elemental_tuning_card_validate.md) 和 [`elemental_tuning_dice_validate`](elemental_tuning_dice_validate.md) 检查。

调和与出牌共享 [`card_count`](card_count.md)、[`card_id`](card_id.md) 提供的手牌候选。无需计算出牌费用，也不采用出牌的支付或目标检查。

下一次 [`step`](../../executor/step.md) 先广播 [`elemental_tuning_modification`](../../../definition/events/elemental_tuning_modification.md)，此时卡牌及附属状态仍然有效。修饰响应结算完成后，按最终 `to` 转换一枚骰子并移除手牌，再广播 [`elemental_tuning_completed`](../../../definition/events/elemental_tuning_completed.md)。转换不触发骰子产生或消耗通知，也不触发出牌原效果及出牌广播。

调和后仍由当前玩家行动，重新处理 [`before_action`](../../../definition/events/before_action.md) 并进入新的行动选择现场。旧现场的候选与报价不再适用。
