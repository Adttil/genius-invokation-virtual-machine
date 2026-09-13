[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<remaining_active_character_selection>](../remaining_active_character_selection.md) / **selected**

# givm::execution_view<execution_state::remaining_active_character_selection>::selected

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr character_id selected() const noexcept;
```
[`character_id`](../../../table/character_id.md)

取得已经接受的第一份角色选择。

## 返回值

先提交一方所选的角色 ID。该选择已经被接受，但尚未写入牌桌的出战角色字段。
