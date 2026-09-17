[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<initial_card_selection>](../initial_card_selection.md) / **selection_validate**

# givm::execution_view<execution_state::initial_card_selection>::selection_validate

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr initial_card_selection_validation selection_validate(
    const table& card_table,
    player_id player,
    std::bitset<selection_capacity> selected
) const noexcept;
```
[`initial_card_selection_validation`](../../initial_card_selection_validation.md)
[`table`](../../../table/table.md)
[`player_id`](../../../table/player_id.md)
[`selection_capacity`](../../selection_capacity.md)

检查首先提交换牌的玩家及其手牌选择是否合法。

## 参数

| | |
| --- | --- |
| `card_table` | 与当前换牌现场配套的牌桌。 |
| `player` | 拟首先提交选择的玩家。 |
| `selected` | 按该玩家当前有效手牌的遍历顺序标记的选择位集。 |

## 返回值

按玩家编号、选中位置的顺序检查，遇到首个失败立即返回；全部通过时返回 `valid`：

| | |
| --- | --- |
| `initial_card_selection_validation::invalid_player` | 玩家编号不是 0 或 1。 |
| `initial_card_selection_validation::invalid_card_position` | 至少一个选中位置不对应该玩家的当前有效手牌。 |
| `initial_card_selection_validation::valid` | 玩家和选中位置均合法。 |

合法玩家的空选择返回 `valid`，表示保留全部手牌。

## 注意

本操作只检查，不填写选择、推进对局或调用随机源。检查成功后仍须调用 [`select`](select.md) 提交；调用方能够保证合法时，也可以直接提交。提交及后续推进不会自动调用本检查。
