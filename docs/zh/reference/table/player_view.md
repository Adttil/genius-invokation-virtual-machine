[givm](../../reference.md) / [牌桌](../table.md) / **player_view**

# givm::player_view

定义于头文件 `<givm/table.hpp>`

```cpp
class player_view;
```

一方玩家的只读视图，用于查看其角色、卡牌、骰子以及场上的持续效果。

## 成员函数

|  |  |
| --- | --- |
| [`id`](player_view/id.md) | 取得实体 ID |
| [`state`](player_view/state.md) | 访问实体状态 |
| [`hand_cards`](player_view/hand_cards.md) | 遍历手牌 |
| [`deck_cards`](player_view/deck_cards.md) | 遍历牌库卡牌 |
| [`supports`](player_view/supports.md) | 遍历支援 |
| [`summons`](player_view/summons.md) | 遍历召唤物 |
| [`combat_statuses`](player_view/combat_statuses.md) | 遍历出战状态 |
| [`characters`](player_view/characters.md) | 遍历角色 |
| [`deck_card_count`](player_view/deck_card_count.md) | 取得牌库张数 |
| [`hand_card_count`](player_view/hand_card_count.md) | 取得手牌张数 |
| [`deck_card_definition`](player_view/deck_card_definition.md) | 取得指定牌库位置的卡牌定义 ID |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个玩家。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。


## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::table table{};
    const givm::player_view player = table[givm::player_id{ 0 }];
    std::println("初始骰子数: {}", player.state().dice.total());
}
```

输出

```text
初始骰子数: 0
```

## 参阅

|  |  |
| --- | --- |
| [`player_state`](player_state.md) | 该实体的状态 |
