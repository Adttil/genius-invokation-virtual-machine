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
| `overflow` | `const bool` | 入手时超出手牌上限，加入后立即失效 |

## 注意

入手时先取得手牌 ID，爆牌立即标记失效，`overflow` 保留该事实。段收尾时保留爆牌记录以及仍在接收方手牌中的记录，之后不因混合事件中的转移或移除再次筛选。事件中的牌 ID 因此可能已经失效；读取前不能假定它仍在手牌中。

抽牌也为爆牌保留本通知，不额外发送 [`hand_card_added`](hand_card_added.md)。需要响应任意方式加入手牌的定义，应同时响应这两种事件；由抽牌触发时只在本次 `card_drawn` 广播中响应一次。

[`draw_cards`](../commands/draw_cards.md) 先完成整批抽取，再按指定顺序逐张结算本通知；首张牌的响应开始时，本批其余牌也已离开牌堆。

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
