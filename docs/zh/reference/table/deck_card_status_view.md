[givm](../../reference.md) / [牌桌](../table.md) / **deck_card_status_view**

# givm::deck_card_status_view

定义于头文件 `<givm/table.hpp>`

```cpp
class deck_card_status_view;
```

附着在一张牌库卡牌上的状态。卡牌从牌库移动到手牌时，可以连同这些状态一起转移。

## 成员函数

|  |  |
| --- | --- |
| [`is_valid`](deck_card_status_view/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](deck_card_status_view/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](deck_card_status_view/size.md) | 取得单实体范围的元素数 |
| [`begin`](deck_card_status_view/begin.md) | 取得单实体范围的起点 |
| [`end`](deck_card_status_view/end.md) | 取得单实体范围的终点 |
| [`player`](deck_card_status_view/player.md) | 取得所属玩家 |
| [`id`](deck_card_status_view/id.md) | 取得实体 ID |
| [`definition_id`](deck_card_status_view/definition_id.md) | 取得实体的定义 ID |
| [`state`](deck_card_status_view/state.md) | 访问实体状态 |
| [`card`](deck_card_status_view/card.md) | 取得所属卡牌 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个牌库卡牌上的状态。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。

## 参阅

|  |  |
| --- | --- |
| [`status_state`](status_state.md) | 该实体的状态 |
