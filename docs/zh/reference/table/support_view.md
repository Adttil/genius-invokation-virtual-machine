[givm](../../reference.md) / [牌桌](../table.md) / **support_view**

# givm::support_view

定义于头文件 `<givm/table.hpp>`

```cpp
class support_view;
```

玩家支援区中一个支援的只读视图。它承载该支援在本场对局中的状态。

## 成员函数

|  |  |
| --- | --- |
| [`is_valid`](support_view/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](support_view/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](support_view/size.md) | 取得单实体范围的元素数 |
| [`begin`](support_view/begin.md) | 取得单实体范围的起点 |
| [`end`](support_view/end.md) | 取得单实体范围的终点 |
| [`player`](support_view/player.md) | 取得所属玩家 |
| [`id`](support_view/id.md) | 取得实体 ID |
| [`definition_id`](support_view/definition_id.md) | 取得实体的定义 ID |
| [`state`](support_view/state.md) | 访问实体状态 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个支援。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。

## 参阅

|  |  |
| --- | --- |
| [`support_state`](support_state.md) | 该实体的状态 |
