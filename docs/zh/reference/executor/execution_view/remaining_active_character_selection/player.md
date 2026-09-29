[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<remaining_active_character_selection>](../remaining_active_character_selection.md) / **player**

# givm::execution_view<execution_state::remaining_active_character_selection>::player

定义于头文件 `<givm/runtime.hpp>`

```cpp
constexpr player_id player() const noexcept(/* Release 为 true，Debug 为 false */);
```
[`player_id`](../../../table/player_id.md)

取得尚待选择出战角色的玩家。

## 返回值

仍需接受出战选择的一方，与 [`first_selected_character()`](first_selected_character.md) 所属玩家不同。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
