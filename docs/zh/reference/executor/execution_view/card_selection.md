[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<card_selection>**

# givm::execution_view<execution_state::card_selection>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::card_selection>;
```

指定玩家换牌现场的视图，包含本次等待换牌的玩家及当前填写的选择。

## 成员函数

| | |
| --- | --- |
| [`player`](card_selection/player.md) | 取得本次等待换牌的玩家。 |
| [`selected`](card_selection/selected.md) | 取得当前填写的换牌选择。 |
| [`check_selection`](card_selection/check_selection.md) | 检查当前玩家的换牌选择是否合法。 |
| [`select`](card_selection/select.md) | 填写要替换的手牌选择。 |

## 注意

本次等待换牌的玩家已经由流程确定，不一定是牌桌上的行动方。选择位按该玩家当前有效手牌的遍历顺序解释，空选择表示保留全部手牌。

此视图用于单方换牌，以及开局双方换牌中剩余一方的选择。尚未接受任何一方的选择时使用 [`execution_view<initial_card_selection>`](initial_card_selection.md)。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
