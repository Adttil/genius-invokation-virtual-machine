[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_reroll_selection>](../dice_reroll_selection.md) / **player**

# givm::execution_view<execution_state::dice_reroll_selection0>::player

定义于头文件 `<givm/runtime.hpp>`

```cpp
static constexpr player_id player() noexcept;
```
[`player_id`](../../../table/player_id.md)

取得本命令指定的重投玩家。

## 返回值

当前等待重投选择的玩家。

## 注意

玩家由状态后缀静态确定。此函数不访问执行现场，也不进行生命周期检查。
