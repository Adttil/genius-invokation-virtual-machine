[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<active_character_changed>**

# givm::execution_view<execution_state::active_character_changed>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::active_character_changed>;
```

一名玩家设置新出战角色前的现场视图。

此时目标角色已经确定，牌桌上仍保留原出战角色及其准备技能附属。下一次推进才实际切换、清除原出战角色的准备技能附属并处理相应通知；这些附属的离场通知先于正常的切换通知。

## 成员函数

| | |
| --- | --- |
| [`character`](active_character_changed/character.md) | 取得本次将设置为出战的角色。 |

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
