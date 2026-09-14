[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **hand_card_created**

# givm::hand_card_created

定义于头文件 `<givm/executor.hpp>`

```cpp
struct hand_card_created;
```

新手牌创建后的通知。事件提供新手牌的标识。

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
    givm::hand_card_created event{ .card = { .player_id = givm::player_id{ 1 }, .index = 2 } };
    std::println("玩家 1 的手牌: {}", event.card.player_id == givm::player_id{ 1 });
    std::println("手牌槽位: {}", event.card.index);
}
```

输出

```text
玩家 1 的手牌: true
手牌槽位: 2
```
