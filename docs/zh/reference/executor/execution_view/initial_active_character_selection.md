[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<initial_active_character_selection>**

# givm::execution_view<execution_state::initial_active_character_selection>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::initial_active_character_selection>;
```

开局尚未接受任何一方出战角色选择时的现场视图。可以任选一方先提交。

## 成员函数

| | |
| --- | --- |
| [`select`](initial_active_character_selection/select.md) | 填写首先提交的出战角色选择。 |

## 注意

首次选择被接受后，出战角色尚未写入牌桌，另一方通过 [`execution_view<remaining_active_character_selection>`](remaining_active_character_selection.md) 提交选择。

## 参阅

| | |
| --- | --- |
| [`select_active_character_both`](../instructions/select_active_character_both.md) | 双方开局出战角色的选择指令 |
