[givm](../../reference.md) / [执行](../executor.md) / **initial_card_selection_check_result**

# givm::initial_card_selection_check_result

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class initial_card_selection_check_result : std::uint8_t
{
    valid,
    invalid_player,
    invalid_card_position
};
```

开局首次换牌选择的检查结果，区分玩家编号非法与所选手牌位置非法。

## 枚举值

| | |
| --- | --- |
| `valid` | 玩家合法，且所有选中位置都对应其当前有效手牌 |
| `invalid_player` | 玩家编号不是 0 或 1 |
| `invalid_card_position` | 至少一个选中位置不对应当前有效手牌 |

## 注意

按玩家编号、选中位置的顺序检查，遇到首个失败立即返回；全部通过时返回 `valid`。合法玩家的空选择返回 `valid`，表示保留全部手牌。

玩家已固定的 `execution_view<card_selection>::check_selection` 只判断手牌位置，返回 `bool`。

## 参阅

| | |
| --- | --- |
| [`execution_view<initial_card_selection>::check_selection`](execution_view/initial_card_selection/check_selection.md) | 检查首次提交的玩家及换牌选择 |
| [`execution_view<card_selection>::check_selection`](execution_view/card_selection/check_selection.md) | 检查指定玩家的换牌选择 |
