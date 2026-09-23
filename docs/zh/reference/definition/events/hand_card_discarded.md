[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **hand_card_discarded**

# givm::hand_card_discarded

定义于头文件 `<givm/definition.hpp>`

```cpp
struct hand_card_discarded
{
    const hand_card_id card;
};
```

[舍弃命令](../commands/discard_hand_card.md)在该牌的 [`hand_card_discard_effect`](hand_card_discard_effect.md) 响应程序执行完成后发送的全场通知。

`card` 是被舍弃牌的强类型 ID。该牌及其附属状态已经离场，不参加此次全场广播；其定义和状态在安全清理前仍可通过 ID 读取。

只有明确的舍弃命令产生此事件。打出牌、元素调和和手牌溢出的移除均不产生舍弃通知。响应全场通知不等于该牌具有自身舍弃效果。
