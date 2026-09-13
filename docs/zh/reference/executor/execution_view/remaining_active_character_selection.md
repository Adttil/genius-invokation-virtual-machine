[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<remaining_active_character_selection>**

# givm::execution_view<execution_state::remaining_active_character_selection>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::remaining_active_character_selection>;
```

开局已接受一方出战选择、等待另一方选择的现场视图。

## 成员函数

| | |
| --- | --- |
| [`player`](remaining_active_character_selection/player.md) | 取得尚待选择出战角色的玩家。 |
| [`selected`](remaining_active_character_selection/selected.md) | 取得已经接受的第一份角色选择。 |
| [`select`](remaining_active_character_selection/select.md) | 填写剩余玩家选择的角色下标。 |

## 注意

此时双方出战角色均尚未写入牌桌。第二份选择被接受后，才同时设置双方结果；以 [`step`](../executor/step.md) 推进时，随后报告 `execution_state::initial_active_characters_selected`。

## 参阅

| | |
| --- | --- |
| [`execution_view<initial_active_character_selection>`](initial_active_character_selection.md) | 开局首次出战选择现场的视图 |
