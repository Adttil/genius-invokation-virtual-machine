[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_view](../deck_card_view.md) / **state**

# givm::deck_card_view::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const card_state& state() const;
```

访问该牌库卡牌在对局中的当前状态。

## 返回值

实体持有的 [`card_state`](../card_state.md) 引用，只允许读取。

## 注意

实体必须有效。
