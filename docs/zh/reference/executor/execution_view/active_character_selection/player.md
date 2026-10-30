[givm](../../../../reference.md) / [执行](../../../executor.md) / [active_character_selection](../active_character_selection.md) / **player**

# givm::execution_view<execution_state::active_character_selection0>::player

定义于头文件 `<givm/runtime.hpp>`。

```cpp
static constexpr player_id player() noexcept;
```

取得需要重新选择出战角色的玩家。

Debug 下视图不属于当前现场或已经失效时抛出 execution_view_error。
