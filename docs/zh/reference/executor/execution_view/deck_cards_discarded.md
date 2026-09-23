[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<deck_cards_discarded>**

# givm::execution_view<execution_state::deck_cards_discarded>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::deck_cards_discarded>;
```

一批牌堆顶的卡牌已经全部舍弃、尚未开始任何自身舍弃效果时的观察视图。仅在 `compile_mode::observed` 下报告；实际舍弃零张牌时不报告。

视图不需要输入。下一次推进从第一张牌的自身效果开始，然后广播其舍弃通知，再依次处理剩余卡牌。

## 成员函数

| | |
| --- | --- |
| [`cards`](deck_cards_discarded/cards.md) | 取得本批被舍弃的牌，按从牌堆顶向下的顺序排列。 |
