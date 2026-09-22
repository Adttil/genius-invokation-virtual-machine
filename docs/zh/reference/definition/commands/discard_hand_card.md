[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **discard_hand_card**

# givm::discard_hand_card

定义于头文件 `<givm/definition.hpp>`

```cpp
struct discard_hand_card;
```

舍弃一张手牌，并触发该牌自身的舍弃效果及全场舍弃通知。舍弃由效果程序执行，不是行动选择阶段可主动提交的操作。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | [`hand_card_id`](../../table/hand_card_id.md) | 固定舍弃目标；默认采用动态输入 |

## 输入

- 默认构造 `discard_hand_card{}`，消费响应通过 `invoke` 提交的一个 [`hand_card_discarded`](../events/hand_card_discarded.md)。
- 显式指定 `card` 时使用该固定目标，不消费响应输入。

执行时目标必须是有效手牌。命令不检查此前提；传入失效目标属于未定义行为。

## 结算

1. 手牌及其附属状态离场，保留信息供本次通知读取。
2. 向这张牌自身发送 `hand_card_discarded`，完整执行其响应程序。
3. 向此时场上的可响应实体广播同一个通知。

自身响应时，这张牌已经不在手牌中；响应仍可通过牌 ID 读取其定义、状态和附属状态信息。被舍弃牌及其附属状态不参加随后的全场广播，因此该牌不会重复响应本次舍弃。

只有本命令执行的操作才属于手牌舍弃。打出牌、元素调和以及超过手牌上限导致的牌离场，都不触发舍弃效果或舍弃通知。

## 示例

```cpp
#include <print>
#include <givm/definition.hpp>

int main()
{
    const givm::hand_card_id card{ givm::player_id{ 0 }, 2 };
    const givm::discard_hand_card command{ .card = card };
    std::println("使用指定手牌: {}", command.card == card);
}
```

输出

```text
使用指定手牌: true
```
