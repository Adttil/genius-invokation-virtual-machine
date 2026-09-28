[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_selection>](../dice_selection.md) / **player**

# givm::execution_view<execution_state::dice_selection>::player

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr player_id player() const noexcept(/* Release 为 true，Debug 为 false */);
```
[`player_id`](../../../table/player_id.md)

取得当前填写的玩家。

## 返回值

当前重投选择对应的玩家。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
