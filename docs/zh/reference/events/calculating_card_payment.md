[givm](../../reference.md) / [事件](../events.md) / **calculating_card_payment**

# givm::calculating_card_payment

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct calculating_card_payment;
```

正在计算打出一张手牌所需的支付内容。事件标识这次要打出的牌。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次事件对应的牌标识；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::calculating_card_payment event{ .card = { .player_id = givm::player_id{ 1 }, .index = 2 } };
    std::println("玩家 1 的手牌: {}", event.card.player_id == givm::player_id{ 1 });
    std::println("手牌槽位: {}", event.card.index);
}
```

输出

```text
玩家 1 的手牌: true
手牌槽位: 2
```
