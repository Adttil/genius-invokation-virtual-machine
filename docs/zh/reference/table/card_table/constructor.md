[givm](../../../reference.md) / [牌桌](../../table.md) / [card_table](../card_table.md) / **(构造函数)**

# givm::card_table::(构造函数)

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr card_table(game_parameters parameters = {});
```

准备一张尚未装载牌组的牌桌，使用指定的对局参数。

双方玩家已经存在；角色和卡牌等实体尚未加入。

## 参数

|  |  |
| --- | --- |
| `parameters` | 对局参数，省略时使用 [`game_parameters`](../game_parameters.md) 的默认值 |

## 返回值

（无）

## 注意

牌桌及其副本拥有各自独立的对局状态，不持有定义库。实体采用的定义由定义 ID 表示。

## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::card_table table{};
    givm::card_table configured{ { .hand_limit = 12 } };
    std::println("手牌上限: {}", configured.parameters().hand_limit);
    std::println("初始回合数: {}", configured.state().round_number);
}
```

输出

```text
手牌上限: 12
初始回合数: 0
```
