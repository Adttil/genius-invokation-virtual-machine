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
| [`select`](dice_selection/select.md) | 填写当前玩家要重投的骰子选择。 |

## 注意

位置按当前骰子种类顺序展开：万能、冰、水、火、雷、风、岩、草，同种骰子逐个计数；重投后应按新组成解释位置。非空选择消耗一次重投机会，空选择放弃该方全部剩余机会。默认玩家只是建议，调用方可以提交任何仍有重投机会的玩家。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
