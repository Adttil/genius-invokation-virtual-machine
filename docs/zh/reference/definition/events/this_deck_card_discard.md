[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **this_deck_card_discard**

# givm::this_deck_card_discard

定义于头文件 `<givm/definition.hpp>`。

```cpp
struct this_deck_card_discard {
    static constexpr event_category category = event_category::normal;
};
```

[discard_deck_cards](../commands/discard_deck_cards.md) 产生的自身舍弃效果事件。仅交给被舍弃的 `deck_card_view`，通过 `context.entity()` 取得该牌，不携带重复的卡牌 ID。卡牌已经无效，定义和状态在安全清理前仍可读取。

命令立即使本批卡牌全部离场。结算时逐张执行自身效果及其完整结算，再向全场发送 [`deck_card_discarded`](deck_card_discarded.md)。全场响应者在自身效果开始前采样，历史摘要在自身效果完成后、全场响应开始前记录本次舍弃。
