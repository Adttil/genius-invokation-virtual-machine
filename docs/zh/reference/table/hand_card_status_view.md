[givm](../../reference.md) / [牌桌](../table.md) / **hand_card_status_view**

# givm::hand_card_status_view

定义于头文件 `<givm/table.hpp>`

```cpp
class hand_card_status_view;
```

附着在一张手牌上的状态，例如跟随该卡牌生效的持续影响。

## 成员函数

|  |  |
| --- | --- |
| [`is_valid`](hand_card_status_view/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](hand_card_status_view/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](hand_card_status_view/size.md) | 取得单实体范围的元素数 |
| [`begin`](hand_card_status_view/begin.md) | 取得单实体范围的起点 |
| [`end`](hand_card_status_view/end.md) | 取得单实体范围的终点 |
| [`player`](hand_card_status_view/player.md) | 取得所属玩家 |
| [`id`](hand_card_status_view/id.md) | 取得实体 ID |
| [`definition_id`](hand_card_status_view/definition_id.md) | 取得实体的定义 ID |
| [`state`](hand_card_status_view/state.md) | 访问实体状态 |
| [`card`](hand_card_status_view/card.md) | 取得所属卡牌 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个手牌上的状态。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。

## 参阅

|  |  |
| --- | --- |
| [`status_state`](status_state.md) | 该实体的状态 |
