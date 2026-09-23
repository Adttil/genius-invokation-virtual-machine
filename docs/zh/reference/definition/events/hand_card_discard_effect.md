[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **hand_card_discard_effect**

# givm::hand_card_discard_effect

定义于头文件 `<givm/definition.hpp>`

```cpp
struct hand_card_discard_effect
{
    const hand_card_id card;
};
```

[舍弃命令](../commands/discard_hand_card.md)仅向被舍弃的手牌自身发送的效果事件。只有 `hand_card_view` 订阅此事件；定义能否响应该事件表示它是否具有从该区域舍弃时的自身效果。

`card` 是已标记离场的卡牌，仍可在安全清理前读取其定义和状态。它也是默认构造的 `discard_hand_card{}` 所需的输入。

自身效果返回的程序全部完成后，才向全场发送 [`hand_card_discarded`](hand_card_discarded.md)。此事件不向卡牌的附属状态或其他实体发送。
