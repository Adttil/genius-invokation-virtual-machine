[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **card_drawn**

# givm::card_drawn

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct card_drawn;
```

一张牌抽取完成后的通知。响应者可以通过牌的标识读取抽到的牌。

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
    givm::card_drawn event{ .card = { .player_id = givm::player_id{ 1 }, .index = 2 } };
    std::println("玩家 1 的手牌: {}", event.card.player_id == givm::player_id{ 1 });
    std::println("手牌槽位: {}", event.card.index);
}
```

输出

```text
玩家 1 的手牌: true
手牌槽位: 2
```

## 参阅

| | |
| --- | --- |
| [`draw_cards`](../instructions/draw_cards.md) | 抽牌指令 |
| [`replace_cards`](../instructions/replace_cards.md) | 单方换牌指令 |
