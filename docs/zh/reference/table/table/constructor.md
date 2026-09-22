[givm](../../../reference.md) / [牌桌](../../table.md) / [table](../table.md) / **(构造函数)**

# givm::table::(构造函数)

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr table(table_state state = {}, player_state player0 = {}, player_state player1 = {});
```

准备一张尚未装载牌组的牌桌，使用指定的牌桌及双方玩家状态。

双方玩家已经存在；角色和卡牌等实体尚未加入。

## 参数

|  |  |
| --- | --- |
| `state` | 共享对局状态，默认采用 [`table_state`](../table_state.md) 的初始值 |
| `player0`、`player1` | 双方玩家状态，默认采用 [`player_state`](../player_state.md) 的初始值 |

## 返回值

（无）

## 注意

[`load_deck`](../../executor/load_deck.md) 保留构造时给定的共享状态和玩家状态。两位玩家的容量限制可分别配置。

牌桌及其副本拥有各自独立的对局状态，不持有定义库。实体采用的定义由定义 ID 表示。

## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::table table{};
    givm::table configured{ {}, { .hand_limit = 12 } };
    std::println("手牌上限: {}", configured[givm::player_id{ 0 }].state().hand_limit);
    std::println("初始回合数: {}", configured.state().round_number);
}
```

输出

```text
手牌上限: 12
初始回合数: 0
```
