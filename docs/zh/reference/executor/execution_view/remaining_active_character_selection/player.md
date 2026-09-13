[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<remaining_active_character_selection>](../remaining_active_character_selection.md) / **player**

# givm::execution_view<execution_state::remaining_active_character_selection>::player

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr player_id player() const noexcept;
```
[`player_id`](../../../table/player_id.md)

取得尚待选择出战角色的玩家。

## 返回值

仍需接受出战选择的一方，与 [`selected()`](selected.md) 所属玩家不同。
