[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<card_selection>](../card_selection.md) / **player**

# givm::execution_view<execution_state::card_selection>::player

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr player_id player() const noexcept(/* Release 为 true，Debug 为 false */);
```
[`player_id`](../../../table/player_id.md)

取得本次等待换牌的玩家。

## 返回值

当前流程确定的待换牌玩家，不一定是牌桌上的行动方。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
