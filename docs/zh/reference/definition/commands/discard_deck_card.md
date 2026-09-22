[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **discard_deck_card**

# givm::discard_deck_card

定义于头文件 `<givm/definition.hpp>`

```cpp
struct discard_deck_card;
```

从牌堆舍弃一张牌，触发该牌自身的舍弃效果及全场通知。它与[舍弃手牌](discard_hand_card.md)使用不同的命令和通知事件。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | [`deck_card_id`](../../table/deck_card_id.md) | 固定舍弃目标；默认采用动态输入 |

## 输入

- 默认构造 `discard_deck_card{}`，消费响应通过 `invoke` 提交的一个 [`deck_card_discarded`](../events/deck_card_discarded.md)。
- 显式指定 `card` 时使用该固定目标，不消费响应输入。

执行时目标必须是有效牌堆牌。命令不检查此前提；传入失效目标属于未定义行为。牌 ID 标识具体实体，不表示牌堆顺序中的第几张。

## 结算

1. 牌及其附属状态离场，保留信息供本次通知读取。
2. 向这张牌自身发送 `deck_card_discarded`，完整执行其响应程序。
3. 向此时场上的可响应实体广播同一个通知。

自身响应时，该牌已不在牌堆中；响应仍可读取其定义、状态和附属状态信息。被舍弃牌及其附属状态不参加随后的全场广播，因此该牌不会重复响应本次舍弃。

只有本命令执行的操作才属于牌堆舍弃。抽牌、转移到手牌或超过手牌上限导致的牌离场，都不产生牌堆舍弃通知。舍弃不是行动选择阶段的主动操作。

## 示例

```cpp
#include <print>
#include <givm/definition.hpp>

int main()
{
    const givm::deck_card_id card{ givm::player_id{ 1 }, 0 };
    const givm::discard_deck_card command{ .card = card };
    std::println("使用指定牌堆牌: {}", command.card == card);
}
```

输出

```text
使用指定牌堆牌: true
```
