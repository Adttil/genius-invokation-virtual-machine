[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<card_selection>](../card_selection.md) / **check_selection**

# givm::execution_view<execution_state::card_selection>::check_selection

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr bool check_selection(
    const table& card_table,
    std::bitset<selection_capacity> selected
) const noexcept;
```
[`table`](../../../table/table.md)
[`selection_capacity`](../../selection_capacity.md)

检查当前等待玩家的换牌选择是否合法。

## 参数

| | |
| --- | --- |
| `card_table` | 与当前换牌现场配套的牌桌。 |
| `selected` | 按 [`player()`](player.md) 的当前有效手牌遍历顺序标记的选择位集。 |

## 返回值

每个选中位置均对应该玩家的当前有效手牌时返回 `true`，否则返回 `false`。空选择返回 `true`，表示保留全部手牌。

## 注意

本操作只检查，不填写选择、推进对局或调用随机源。检查成功后仍须调用 [`select`](select.md) 提交；调用方能够保证合法时，也可以直接提交。提交及后续推进不会自动调用本检查。
