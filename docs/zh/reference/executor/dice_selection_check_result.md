[givm](../../reference.md) / [执行](../executor.md) / **dice_selection_check_result**

# givm::dice_selection_check_result

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class dice_selection_check_result : std::uint8_t
{
    valid,
    invalid_player,
    no_rerolls_remaining,
    insufficient_dice
};
```

显式指定玩家调用 `check_selection(card_table, player, selected)` 的检查结果，区分玩家编号非法、没有重投机会和持有骰子不足。

## 枚举值

| | |
| --- | --- |
| `valid` | 玩家合法、仍有重投机会，且持有所选骰子 |
| `invalid_player` | 玩家编号不是 0 或 1 |
| `no_rerolls_remaining` | 该玩家已经没有重投机会 |
| `insufficient_dice` | 至少一种所选骰子的数量超过当前持有数量 |

## 注意

按玩家编号、剩余重投机会、持有数量的顺序检查，遇到首个失败立即返回；全部通过时返回 `valid`。

所有数量为零的选择表示放弃该方全部剩余重投机会；合法玩家仍有机会时返回 `valid`。

不指定玩家的重载沿用当前玩家，只需检查持有数量，返回 `bool`。

## 参阅

| | |
| --- | --- |
| [`execution_view<dice_selection>::check_selection`](execution_view/dice_selection/check_selection.md) | 检查骰子重投选择 |
