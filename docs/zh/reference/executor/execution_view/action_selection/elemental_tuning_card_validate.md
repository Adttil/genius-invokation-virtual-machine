[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **elemental_tuning_card_validate**

# givm::execution_view<execution_state::action_selection>::elemental_tuning_card_validate

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr bool elemental_tuning_card_validate(const table& card_table, std::size_t card_index) const noexcept;
```

检查指定手牌是否允许用于元素调和。

## 参数

| | |
| --- | --- |
| `card_table` | 当前行动发生的牌桌。 |
| `card_index` | 当前手牌候选索引，须小于 [`card_count()`](card_count.md)。 |

## 返回值

该牌 `state().elemental_tuning_allowed` 的值。`true` 表示允许调和。

## 注意

与出牌使用同一组手牌候选，无需先计算费用。本操作只检查卡牌许可，不检查选中的骰子，也不提交行动。
