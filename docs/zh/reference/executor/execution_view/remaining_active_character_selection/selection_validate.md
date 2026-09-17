[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<remaining_active_character_selection>](../remaining_active_character_selection.md) / **selection_validate**

# givm::execution_view<execution_state::remaining_active_character_selection>::selection_validate

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr remaining_active_character_selection_validation selection_validate(
    const table& card_table,
    character_id character
) const noexcept;
```
[`remaining_active_character_selection_validation`](../../remaining_active_character_selection_validation.md)
[`table`](../../../table/table.md)
[`character_id`](../../../table/character_id.md)

检查角色是否可作为剩余一方的开局出战选择。

## 参数

| | |
| --- | --- |
| `card_table` | 与当前出战选择现场配套的牌桌。 |
| `character` | 拟选择的完整角色 ID。 |

## 返回值

按所属玩家编号、是否为待选玩家、角色是否存在且有效的顺序检查，遇到首个失败立即返回；全部通过时返回 `valid`：

| | |
| --- | --- |
| `remaining_active_character_selection_validation::invalid_player` | 所属玩家编号不是 0 或 1。 |
| `remaining_active_character_selection_validation::wrong_player` | 所属玩家合法，但不是 [`player()`](player.md) 指定的待选玩家。 |
| `remaining_active_character_selection_validation::invalid_character` | 角色下标越界，或对应角色已被移除。 |
| `remaining_active_character_selection_validation::valid` | 角色属于待选玩家，且存在并有效。 |

角色生命值不属于这项检查的条件。

## 注意

本操作只检查，不填写选择、推进对局或调用随机源。检查成功后仍须调用 [`select`](select.md) 提交；调用方能够保证合法时，也可以直接提交。提交及后续推进不会自动调用本检查。
