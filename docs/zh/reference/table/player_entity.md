[givm](../../reference.md) / [牌桌](../table.md) / **player_entity**

# givm::player_entity

定义于头文件 `<givm/table.hpp>`

```cpp
template<class TStorage>
class player_entity;
```

一方玩家的访问入口，用于查看和调整其角色、卡牌、骰子以及场上的持续效果。

## 模板参数

|  |  |
| --- | --- |
| `TStorage` | 由牌桌决定的存储类型；是否具有 const 限定决定实体能否修改。通常通过牌桌取得对象并使用 `auto`，无需显式指定此参数 |

## 成员类型

|  |  |
| --- | --- |
| `table_type` | `TStorage` |
| `data_type` | 实体数据类型，随 `is_mutable` 保留只读限定 |
| `storage_type` | 实体访问所用的辅助类型 |

## 成员常量

|  |  |
| --- | --- |
| [`is_mutable`](player_entity/is_mutable.md) | 实体是否允许修改 |

## 成员函数

|  |  |
| --- | --- |
| [`operator player_view`](player_entity/conversion.md) | 取得同一实体的只读视图 |
| [`id`](player_entity/id.md) | 取得实体 ID |
| [`state`](player_entity/state.md) | 访问实体状态 |
| [`hand_cards`](player_entity/hand_cards.md) | 遍历手牌 |
| [`deck_cards`](player_entity/deck_cards.md) | 遍历牌库卡牌 |
| [`supports`](player_entity/supports.md) | 遍历支援 |
| [`summons`](player_entity/summons.md) | 遍历召唤物 |
| [`combat_statuses`](player_entity/combat_statuses.md) | 遍历出战状态 |
| [`characters`](player_entity/characters.md) | 遍历角色 |
| [`add_hand_card`](player_entity/add_hand_card.md) | 将卡牌加入手牌 |
| [`add_deck_card`](player_entity/add_deck_card.md) | 将新卡牌加入牌库顶 |
| [`insert_deck_card`](player_entity/insert_deck_card.md) | 向指定牌库位置插牌 |
| [`deck_card_count`](player_entity/deck_card_count.md) | 取得牌库张数 |
| [`swap_deck_cards`](player_entity/swap_deck_cards.md) | 交换两张牌的牌库位置 |
| [`hand_card_count`](player_entity/hand_card_count.md) | 取得手牌张数 |
| [`deck_card_definition`](player_entity/deck_card_definition.md) | 取得指定牌库位置的卡牌定义 ID |
| [`take_top_deck_card`](player_entity/take_top_deck_card.md) | 从牌库顶取出卡牌 |
| [`take_deck_cards`](player_entity/take_deck_cards.md) | 从指定牌库位置取出卡牌 |
| [`take_hand_card`](player_entity/take_hand_card.md) | 从手牌中取出卡牌 |
| [`discard_top_deck_card`](player_entity/discard_top_deck_card.md) | 丢弃牌库顶的卡牌 |
| [`add`](player_entity/add.md) | 增加角色、支援、召唤物或出战状态 |
| [`clean_up`](player_entity/clean_up.md) | 清理该玩家已移除的实体 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个玩家。访问对象的存活、只读转换和移除约定见[实体的身份与访问](entity_access.md)。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto entity = table[givm::player_id{ 0 }];
    std::println("骰子数: {}", entity.state().dice.total());
}
```

输出

```text
骰子数: 0
```

## 参阅

|  |  |
| --- | --- |
| [`player_view`](player_view.md) | 对应的只读视图 |
| [`player_state`](player_state.md) | 该实体的状态 |
