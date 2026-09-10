[givm](../../reference.md) / [牌桌](../table.md) / **card_table**

# givm::card_table

定义于头文件 `<givm/table.hpp>`

```cpp
class card_table;
```

一场游戏的牌桌。

它记录双方玩家的手牌、牌库、角色、骰子，以及支援、召唤物等持续影响对局的实体和状态。

## 成员类型

|  |  |
| --- | --- |
| `definition_library_type` | [`definition_library`](../definition/definition_library.md) |
| `game_state` | [`table_state`](table_state.md) |

## 成员函数

|  |  |
| --- | --- |
| [`(构造函数)`](card_table/constructor.md) | 构造一个 `card_table` |
| [`definition_library`](card_table/definition_library.md) | 取得配套定义库 |
| [`parameters`](card_table/parameters.md) | 访问对局参数 |
| [`state`](card_table/state.md) | 访问对局的共同状态 |
| [`players`](card_table/players.md) | 遍历双方玩家 |
| [`operator[]`](card_table/operator_subscript.md) | 通过实体 ID 访问实体 |
| [`load_deck`](card_table/load_deck.md) | 为玩家装载牌组 |
| [`clean_up`](card_table/clean_up.md) | 清理已经移除的实体 |

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    table[givm::player_id{ 0 }].state().dice[givm::elemental_dice::omni] = 3;
    for(const auto player : table.players())
    {
        std::println("玩家 {} 的骰子数: {}", player.id().index, player.state().dice.total());
    }
}
```

输出

```text
玩家 0 的骰子数: 3
玩家 1 的骰子数: 0
```

## 参阅

|  |  |
| --- | --- |
| [实体的身份与访问](entity_access.md) | ID、实体访问对象和只读视图的使用约定 |
| [`executor`](../executor/executor.md) | 游戏对局的执行器 |
