[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<initial_card_selection>**

# givm::execution_view<execution_state::initial_card_selection>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::initial_card_selection>;
```

开局尚未接受任何一方换牌选择时的现场视图。可以任选一方先提交。

## 成员函数

| | |
| --- | --- |
| [`selection_validate`](initial_card_selection/selection_validate.md) | 检查指定玩家的换牌选择是否合法。 |
| [`select`](initial_card_selection/select.md) | 指定玩家并填写其换牌选择。 |

## 注意

首次选择被接受并完成换牌后，另一方的输入通过 [`execution_view<card_selection>`](card_selection.md) 提交，该玩家已经固定。

## 参阅

| | |
| --- | --- |
| [`replace_cards_both`](../../definition/commands/replace_cards_both.md) | 双方开局换牌命令 |
