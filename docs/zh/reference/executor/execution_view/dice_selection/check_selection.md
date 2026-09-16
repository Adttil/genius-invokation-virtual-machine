[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_selection>](../dice_selection.md) / **check_selection**

# givm::execution_view<execution_state::dice_selection>::check_selection

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr bool check_selection(
    const table& card_table,
    const dice_counts& selected
) const noexcept;
constexpr dice_selection_check_result check_selection(
    const table& card_table,
    player_id player,
    const dice_counts& selected
) const noexcept;
```
[`dice_selection_check_result`](../../dice_selection_check_result.md)
[`table`](../../../table/table.md)
[`player_id`](../../../table/player_id.md)
[`dice_counts`](../../../enums/dice_counts.md)

检查重投选择。第一个重载沿用 [`player()`](player.md)，只检查是否持有准备重投的骰子；第二个重载同时检查指定玩家是否合法及是否仍可重投。

## 参数

| | |
| --- | --- |
| `card_table` | 与当前重投现场配套的牌桌。 |
| `player` | 拟提交重投选择的玩家。 |
| `selected` | 拟重投的各类骰子数量。 |

## 返回值

不指定玩家的重载中，当前玩家已经确定且仍有重投机会。各类所选骰子均不超过其当前持有数量时返回 `true`，否则返回 `false`。

显式指定玩家的重载按玩家编号、剩余重投机会、持有数量的顺序检查，遇到首个失败立即返回；全部通过时返回 `valid`：

| | |
| --- | --- |
| `dice_selection_check_result::invalid_player` | 玩家编号不是 0 或 1。 |
| `dice_selection_check_result::no_rerolls_remaining` | 该玩家已经没有重投机会。 |
| `dice_selection_check_result::insufficient_dice` | 至少一种所选骰子的数量超过该玩家当前持有数量。 |
| `dice_selection_check_result::valid` | 玩家合法、仍有重投机会，且持有所选骰子。 |

所有数量均为零的选择表示放弃该方全部剩余机会。不指定玩家时返回 `true`；显式指定合法且仍有机会的玩家时返回 `valid`。

## 注意

本操作只检查，不填写选择、消耗重投机会、推进对局或调用随机源。检查成功后仍须调用 [`select`](select.md) 提交；调用方能够保证合法时，也可以直接提交。提交及后续推进不会自动调用本检查。
