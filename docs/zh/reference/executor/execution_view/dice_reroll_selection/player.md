[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_reroll_selection>](../dice_reroll_selection.md) / **player**

# givm::execution_view<execution_state::dice_reroll_selection>::player

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr player_id player() const noexcept;
```
[`player_id`](../../../table/player_id.md)

取得本命令指定的重投玩家。

## 返回值

当前等待重投选择的玩家。
