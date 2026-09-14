[givm](../../reference.md) / [牌桌](../table.md) / **skill_view**

# givm::skill_view

定义于头文件 `<givm/table.hpp>`

```cpp
class skill_view;
```

一名角色所拥有技能的只读视图。它承载该技能在本场对局中的状态。

## 成员函数

|  |  |
| --- | --- |
| [`is_valid`](skill_view/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](skill_view/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](skill_view/size.md) | 取得单实体范围的元素数 |
| [`begin`](skill_view/begin.md) | 取得单实体范围的起点 |
| [`end`](skill_view/end.md) | 取得单实体范围的终点 |
| [`player`](skill_view/player.md) | 取得所属玩家 |
| [`id`](skill_view/id.md) | 取得实体 ID |
| [`definition_id`](skill_view/definition_id.md) | 取得实体的定义 ID |
| [`state`](skill_view/state.md) | 访问实体状态 |
| [`character`](skill_view/character.md) | 取得所属角色 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个技能。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。

## 参阅

|  |  |
| --- | --- |
| [`skill_state`](skill_state.md) | 该实体的状态 |
