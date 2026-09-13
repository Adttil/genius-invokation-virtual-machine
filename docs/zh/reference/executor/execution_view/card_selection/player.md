[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<card_selection>](../card_selection.md) / **player**

# givm::execution_view<execution_state::card_selection>::player

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr player_id player() const noexcept;
```
[`player_id`](../../../table/player_id.md)

取得本次等待换牌的玩家。

## 返回值

当前流程确定的待换牌玩家，不一定是牌桌上的行动方。
