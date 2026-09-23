[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view](../../execution_view.md) / [deck_cards_discarded](../deck_cards_discarded.md) / **cards**

# cards

```cpp
constexpr std::span<const deck_card_id> cards() const noexcept;
```

返回本批全部被舍弃卡牌的 ID，顺序为原牌堆从顶向下的顺序，也就是随后自身效果和通知的结算顺序。

这些牌已全部离场；可用 ID 从牌桌读取其定义和状态。返回的借用在下一次推进或重建执行现场后失效。
