[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<dice_selection>**

# givm::execution_view<execution_state::dice_selection>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::dice_selection>;
```

骰子重投选择现场的视图，包含玩家、当前选择及剩余重投机会。

## 成员函数

| | |
| --- | --- |
| [`player`](dice_selection/player.md) | 取得当前填写的玩家。 |
| [`selected`](dice_selection/selected.md) | 取得当前填写的重投选择。 |
| [`remaining`](dice_selection/remaining.md) | 取得当前玩家剩余的重投次数。 |
| [`dice_count`](dice_selection/dice_count.md) | 取得本次投骰阶段的骰子数量。 |
| [`check_selection`](dice_selection/check_selection.md) | 检查玩家是否还有重投机会，且持有所选骰子。 |
| [`select`](dice_selection/select.md) | 填写当前玩家要重投的骰子选择。 |

## 注意

选择用 [`dice_counts`](../../enums/dice_counts.md) 表示每种骰子要重投的数量，各类数量不得超过该玩家当前持有的数量。非空选择消耗一次重投机会，空选择放弃该方全部剩余机会。默认玩家只是建议，调用方可以提交任何仍有重投机会的玩家。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
