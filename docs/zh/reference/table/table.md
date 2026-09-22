[givm](../../reference.md) / [牌桌](../table.md) / **table**

# givm::table

定义于头文件 `<givm/table.hpp>`

```cpp
class table;
```

一场游戏的牌桌。

它记录双方玩家的手牌、牌库、角色、骰子，以及支援、召唤物等持续影响对局的实体和状态。牌桌不持有定义库，实体通过定义 ID 标识所采用的定义。公开访问提供只读视图，对局中的状态变化通过执行器完成。

## 成员类型

|  |  |
| --- | --- |
| `game_state` | [`table_state`](table_state.md) |

## 成员函数

|  |  |
| --- | --- |
| [`(构造函数)`](table/constructor.md) | 构造一个 `table` |
| [`state`](table/state.md) | 访问对局的共同状态 |
| [`players`](table/players.md) | 遍历双方玩家 |
| [`operator[]`](table/operator_subscript.md) | 通过实体 ID 访问实体 |
| [`clean_up`](table/clean_up.md) | 清理已经移除的实体 |


## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::table table{};
    for(const auto player : table.players())
    {
        std::println("玩家 {} 的骰子数: {}", player.id().index, player.state().dice.total());
    }
}
```

输出

```text
玩家 0 的骰子数: 0
玩家 1 的骰子数: 0
```

## 参阅

|  |  |
| --- | --- |
| [实体的身份与访问](entity_access.md) | ID、实体访问对象和只读视图的使用约定 |
| [`load_deck`](../executor/load_deck.md) | 装载牌组并初始化角色 |
| [`executor`](../executor/executor.md) | 游戏对局的执行器 |
