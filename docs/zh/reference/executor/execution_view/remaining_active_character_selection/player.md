[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<remaining_active_character_selection>](../remaining_active_character_selection.md) / **player**

# givm::execution_view<execution_state::remaining_active_character_selection0>::player

定义于头文件 `<givm/runtime.hpp>`

```cpp
static constexpr player_id player() noexcept;
```
[`player_id`](../../../table/player_id.md)

取得尚待选择出战角色的玩家。

## 返回值

仍需接受出战选择的一方，与 [`first_selected_character()`](first_selected_character.md) 所属玩家不同。

## 注意

玩家由状态后缀静态确定。此函数不访问执行现场，也不进行生命周期检查。
