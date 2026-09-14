[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_view](../deck_card_view.md) / **statuses**

# givm::deck_card_view::statuses

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto statuses() const;
```

遍历附着在这张卡牌上的状态，按添加顺序取得尚未移除的状态。

## 返回值

产生 [`deck_card_status_view`](../deck_card_status_view.md) 的范围。

## 注意

卡牌必须有效。移除或增加状态后应重新取得范围；不要继续使用受修改影响的遍历位置。
