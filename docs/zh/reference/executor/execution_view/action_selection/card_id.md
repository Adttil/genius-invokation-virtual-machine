[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **card_id**

# givm::execution_view<execution_state::action_selection>::card_id

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr hand_card_id card_id(std::size_t card_index) const noexcept;
```
[`hand_card_id`](../../../table/hand_card_id.md)

按出牌候选索引取得对应的手牌 ID，供上层查询牌桌并显示这张牌的信息。

## 参数

| | |
| --- | --- |
| `card_index` | 从零开始的出牌候选索引，须小于 [`card_count()`](card_count.md)。 |

## 返回值

指定出牌候选的手牌 ID。

## 注意

无需先计算费用。本操作不提交行动，也不修改牌桌或推进执行器。

候选索引仅用于当前行动现场。进入下一次行动现场后，须重新取得候选范围，不能假设同一索引仍指向原来的牌。

## 参阅

[`begin_action` 示例](../../../definition/commands/begin_action.md#示例)演示如何从候选索引取得 ID 并读取牌桌信息。
