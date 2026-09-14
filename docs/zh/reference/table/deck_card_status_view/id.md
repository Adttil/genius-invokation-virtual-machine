[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_status_view](../deck_card_status_view.md) / **id**

# givm::deck_card_status_view::id

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr deck_card_status_id id() const;
```

取得牌库卡牌上的状态的身份，以便之后从牌桌再次访问它。

## 返回值

该实体的 [`deck_card_status_id`](../deck_card_status_id.md)。

## 注意

实体必须有效。ID 的保存期限见[实体的身份与访问](../entity_access.md)。
