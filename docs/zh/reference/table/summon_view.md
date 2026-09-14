[givm](../../reference.md) / [牌桌](../table.md) / **summon_view**

# givm::summon_view

定义于头文件 `<givm/table.hpp>`

```cpp
class summon_view;
```

玩家召唤区中一个召唤物的只读视图。它承载这个召唤物在本场对局中的状态。

## 成员函数

|  |  |
| --- | --- |
| [`is_valid`](summon_view/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](summon_view/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](summon_view/size.md) | 取得单实体范围的元素数 |
| [`begin`](summon_view/begin.md) | 取得单实体范围的起点 |
| [`end`](summon_view/end.md) | 取得单实体范围的终点 |
| [`player`](summon_view/player.md) | 取得所属玩家 |
| [`id`](summon_view/id.md) | 取得实体 ID |
| [`definition_id`](summon_view/definition_id.md) | 取得实体的定义 ID |
| [`state`](summon_view/state.md) | 访问实体状态 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个召唤物。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。

## 参阅

|  |  |
| --- | --- |
| [`summon_state`](summon_state.md) | 该实体的状态 |
