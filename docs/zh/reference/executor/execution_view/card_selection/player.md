[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<card_selection>](../card_selection.md) / **player**

# givm::execution_view<execution_state::card_selection0>::player

定义于头文件 `<givm/runtime.hpp>`

```cpp
static constexpr player_id player() noexcept;
```
[`player_id`](../../../table/player_id.md)

取得本次等待换牌的玩家。

## 返回值

当前流程确定的待换牌玩家，不一定是牌桌上的行动方。

## 注意

玩家由状态后缀静态确定。此函数不访问执行现场，也不进行生命周期检查。
