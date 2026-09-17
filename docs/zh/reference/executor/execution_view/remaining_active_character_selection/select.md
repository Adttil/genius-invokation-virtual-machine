[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<remaining_active_character_selection>](../remaining_active_character_selection.md) / **select**

# givm::execution_view<execution_state::remaining_active_character_selection>::select

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void select(character_id character) const noexcept;
```
[`character_id`](../../../table/character_id.md)

填写剩余玩家选择的角色 ID。

## 参数

| | |
| --- | --- |
| `character` | [`player()`](player.md) 对应玩家的一名有效角色的完整 ID |

## 返回值

(无)

## 注意

所属玩家由当前现场确定。可先通过 [`selection_validate`](selection_validate.md) 独立检查；本操作不自动检查，只填写选择，下一次推进才同时设置双方的出战角色。
