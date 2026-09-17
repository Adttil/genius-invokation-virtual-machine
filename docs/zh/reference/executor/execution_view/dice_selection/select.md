[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_selection>](../dice_selection.md) / **select**

# givm::execution_view<execution_state::dice_selection>::select

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void select(const dice_counts& selected) const noexcept;
constexpr void select(player_id player, const dice_counts& selected) const noexcept;
```
[`player_id`](../../../table/player_id.md)
[`dice_counts`](../../../enums/dice_counts.md)

填写要重投的骰子选择。第一个重载沿用当前玩家，第二个重载同时指定玩家。

## 参数

| | |
| --- | --- |
| `player` | 本次提交选择的玩家，必须仍有重投机会。 |
| `selected` | 要重投的各类骰子数量，不得超过该方当前持有的数量；全零表示放弃该方所有剩余重投机会。 |

## 返回值

(无)

## 注意

可先通过 [`selection_validate`](selection_validate.md) 独立检查。本操作不自动检查，只填写选择，下一次推进才消费输入。
