[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_selection>](../dice_selection.md) / **select**

# givm::execution_view<execution_state::dice_selection>::select

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void select(std::bitset<selection_capacity> selected) const noexcept;
constexpr void select(player_id player, std::bitset<selection_capacity> selected) const noexcept;
```
[`player_id`](../../../table/player_id.md)
[`selection_capacity`](../../selection_capacity.md)

填写要重投的骰子选择。第一个重载沿用当前玩家，第二个重载同时指定玩家。

## 参数

| | |
| --- | --- |
| `player` | 本次提交选择的玩家，必须仍有重投机会。 |
| `selected` | 按当前骰子组成展开的选择位集；空选择表示放弃该方所有剩余重投机会。 |

## 返回值

(无)

## 注意

本操作只填写选择，下一次推进才消费输入。
