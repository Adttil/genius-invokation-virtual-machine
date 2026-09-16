[givm](../../reference.md) / [执行](../executor.md) / **initial_active_character_selection_check_result**

# givm::initial_active_character_selection_check_result

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class initial_active_character_selection_check_result : std::uint8_t
{
    valid,
    invalid_player,
    invalid_character
};
```

开局首次出战角色选择的检查结果，区分玩家编号非法与角色无效。

## 枚举值

| | |
| --- | --- |
| `valid` | 玩家合法，且角色存在并有效 |
| `invalid_player` | 角色所属玩家编号不是 0 或 1 |
| `invalid_character` | 角色下标越界，或对应角色已被移除 |

## 注意

任意一方都可以先提交。按玩家编号、角色是否存在且有效的顺序检查，遇到首个失败立即返回；全部通过时返回 `valid`。

角色生命值不属于这项检查的条件。

## 参阅

| | |
| --- | --- |
| [`execution_view<initial_active_character_selection>::check_selection`](execution_view/initial_active_character_selection/check_selection.md) | 检查首次出战角色选择 |
