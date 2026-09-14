[givm](../../reference.md) / [牌桌](../table.md) / **attachment_view**

# givm::attachment_view

定义于头文件 `<givm/table.hpp>`

```cpp
class attachment_view;
```

角色身上一个附属实体的只读视图，例如随角色持续存在的效果或装备。具体用途由定义决定。

## 成员函数

|  |  |
| --- | --- |
| [`is_valid`](attachment_view/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](attachment_view/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](attachment_view/size.md) | 取得单实体范围的元素数 |
| [`begin`](attachment_view/begin.md) | 取得单实体范围的起点 |
| [`end`](attachment_view/end.md) | 取得单实体范围的终点 |
| [`player`](attachment_view/player.md) | 取得所属玩家 |
| [`id`](attachment_view/id.md) | 取得实体 ID |
| [`definition_id`](attachment_view/definition_id.md) | 取得实体的定义 ID |
| [`state`](attachment_view/state.md) | 访问实体状态 |
| [`character`](attachment_view/character.md) | 取得所属角色 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个角色附属实体。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。

## 参阅

|  |  |
| --- | --- |
| [`attachment_state`](attachment_state.md) | 该实体的状态 |
