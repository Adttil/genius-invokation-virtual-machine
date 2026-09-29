[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<dice_reroll_selection>**

# givm::execution_view<execution_state::dice_reroll_selection>

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<>
class execution_view<execution_state::dice_reroll_selection>;
```

[`reroll_dice`](../../definition/commands/reroll_dice.md) 命令等待指定玩家选择重投骰子的现场视图。

## 成员函数

| | |
| --- | --- |
| [`player`](dice_reroll_selection/player.md) | 取得进行重投的玩家。 |
| [`selection_validate`](dice_reroll_selection/selection_validate.md) | 检查玩家是否持有所选骰子。 |
| [`select`](dice_reroll_selection/select.md) | 填写本次要重投的骰子选择。 |

## 注意

玩家由命令确定。选择用 [`dice_counts`](../../enums/dice_counts.md) 表示每种骰子要重投的数量，各类数量不得超过该玩家当前持有数量。非空选择消耗一次重投机会，空选择放弃全部剩余机会。

每次输入直接提交并推进；独立检查不提交输入。Debug 提交会自动检查，Release 由调用方保证输入合法。随机结果在首次等待输入前已经取得，具体规则见 [`reroll_dice`](../../definition/commands/reroll_dice.md)。

## 参阅

| | |
| --- | --- |
| [`execution_view<dice_selection>`](dice_selection.md) | 双方投骰阶段的重投选择现场视图 |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
