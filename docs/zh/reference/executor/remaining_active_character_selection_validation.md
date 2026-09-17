[givm](../../reference.md) / [执行](../executor.md) / **remaining_active_character_selection_validation**

# givm::remaining_active_character_selection_validation

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class remaining_active_character_selection_validation : std::uint8_t
{
    valid,
    invalid_player,
    wrong_player,
    invalid_character
};
```

开局剩余一方的出战角色选择检查结果，区分玩家编号非法、选择了另一方的角色和角色无效。

## 枚举值

| | |
| --- | --- |
| `valid` | 角色属于待选玩家，且存在并有效 |
| `invalid_player` | 角色所属玩家编号不是 0 或 1 |
| `wrong_player` | 角色所属玩家合法，但不是当前等待选择的玩家 |
| `invalid_character` | 角色下标越界，或对应角色已被移除 |

## 注意

按玩家编号、是否为待选玩家、角色是否存在且有效的顺序检查，遇到首个失败立即返回；全部通过时返回 `valid`。

角色生命值不属于这项检查的条件。

## 参阅

| | |
| --- | --- |
| [`execution_view<remaining_active_character_selection>::selection_validate`](execution_view/remaining_active_character_selection/selection_validate.md) | 检查剩余一方的出战角色选择 |
