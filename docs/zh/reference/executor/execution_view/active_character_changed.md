[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<active_character_changed>**

# givm::execution_view<execution_state::active_character_changed>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::active_character_changed>;
```

一名玩家设置新出战角色前的现场视图。

此时目标角色已经确定，牌桌上仍保留原出战角色。下一次推进才写入目标，并处理相应变更响应。

## 成员函数

| | |
| --- | --- |
| [`character`](active_character_changed/character.md) | 取得本次将设置为出战的角色。 |

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
