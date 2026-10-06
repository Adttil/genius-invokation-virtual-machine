[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **hand_card_discard_effect**

# givm::hand_card_discard_effect

定义于头文件 `<givm/definition.hpp>`。

```cpp
struct hand_card_discard_effect {};
```

[discard_hand_card](../commands/discard_hand_card.md) 产生的自身舍弃效果事件。仅交给被舍弃的 `hand_card_view`，通过 `context.entity()` 取得该牌，不携带重复的卡牌 ID。卡牌已经无效，定义和状态在安全清理前仍可读取。

命令立即使本批卡牌全部离场。结算时逐张执行自身效果及其完整结算，再向全场发送 [`hand_card_discarded`](hand_card_discarded.md)。全场响应者在自身效果开始前采样，历史摘要在自身效果完成后、全场响应开始前记录本次舍弃。
