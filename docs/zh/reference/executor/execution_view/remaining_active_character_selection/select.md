[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<remaining_active_character_selection>](../remaining_active_character_selection.md) / **select**

# givm::execution_view<execution_state::remaining_active_character_selection>::select

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void select(std::size_t index) const noexcept;
```

填写剩余玩家选择的角色下标。

## 参数

| | |
| --- | --- |
| `index` | [`player()`](player.md) 对应玩家的一名有效角色的下标，即其角色 ID 的 `index` 字段 |

## 返回值

(无)

## 注意

所属玩家由当前现场确定。本操作只填写选择，下一次推进才同时设置双方的出战角色。
