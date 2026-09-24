[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_drawn**

# givm::card_drawn

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_drawn;
```

一张牌抽取完成后的通知。响应者可以通过牌的标识读取抽到的牌。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次事件对应的牌标识；只读 |

## 注意

抽牌只对实际进入手牌的牌发出本通知，不额外发送 [`hand_card_added`](hand_card_added.md)。需要响应任意方式加入手牌的定义，应同时响应这两种事件；由抽牌触发时只在本次 `card_drawn` 广播中响应一次。

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
| [`draw_cards`](../commands/draw_cards.md) | 抽牌命令 |
| [`replace_cards`](../commands/replace_cards.md) | 单方换牌命令 |
| [`hand_card_added`](hand_card_added.md) | 非抽牌方式加入手牌后的通知 |
