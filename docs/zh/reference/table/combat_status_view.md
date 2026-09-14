[givm](../../reference.md) / [牌桌](../table.md) / **combat_status_view**

# givm::combat_status_view

定义于头文件 `<givm/table.hpp>`

```cpp
class combat_status_view;
```

一方出战状态的只读视图。它属于玩家，可在出战角色变化后继续存在。

## 成员函数

|  |  |
| --- | --- |
| [`is_valid`](combat_status_view/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](combat_status_view/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](combat_status_view/size.md) | 取得单实体范围的元素数 |
| [`begin`](combat_status_view/begin.md) | 取得单实体范围的起点 |
| [`end`](combat_status_view/end.md) | 取得单实体范围的终点 |
| [`player`](combat_status_view/player.md) | 取得所属玩家 |
| [`id`](combat_status_view/id.md) | 取得实体 ID |
| [`definition_id`](combat_status_view/definition_id.md) | 取得实体的定义 ID |
| [`state`](combat_status_view/state.md) | 访问实体状态 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个出战状态。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。

## 参阅

|  |  |
| --- | --- |
| [`combat_status_state`](combat_status_state.md) | 该实体的状态 |
