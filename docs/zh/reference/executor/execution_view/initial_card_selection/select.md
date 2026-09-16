[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<initial_card_selection>](../initial_card_selection.md) / **select**

# givm::execution_view<execution_state::initial_card_selection>::select

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void select(player_id player, std::bitset<selection_capacity> selected) const noexcept;
```
[`player_id`](../../../table/player_id.md)
[`selection_capacity`](../../selection_capacity.md)

指定首先提交换牌的玩家，并填写其换牌选择。

## 参数

| | |
| --- | --- |
| `player` | 首先提交换牌选择的玩家，可任选一方 |
| `selected` | 按该玩家当前有效手牌遍历顺序标记的选择位集，空选择表示全部保留 |

## 返回值

(无)

## 注意

可先通过 [`check_selection`](check_selection.md) 独立检查。本操作只填写输入，下一次推进才执行换牌；不自动检查位集或现场是否合法。
