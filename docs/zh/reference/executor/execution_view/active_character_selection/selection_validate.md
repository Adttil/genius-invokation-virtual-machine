[givm](../../../../reference.md) / [执行](../../../executor.md) / [active_character_selection](../active_character_selection.md) / **selection_validate**

# givm::execution_view<execution_state::active_character_selection>::selection_validate

定义于头文件 `<givm/runtime.hpp>`。

```cpp
active_character_selection_validation selection_validate(
    const table& table, character_id character) const noexcept(/* Release 为 true，Debug 为 false */);
```

检查玩家有效、与 player() 一致，角色有效且 alive、生命非零。返回 valid、invalid_player、wrong_player、invalid_character 或 defeated_character；不会修改牌桌。

Debug 下视图不属于当前现场或已经失效时抛出 execution_view_error。
