[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_id](../deck_card_id.md) / **operator==**

# givm::deck_card_id::operator==

定义于头文件 `<givm/table.hpp>`

```cpp
friend constexpr bool operator==(deck_card_id, deck_card_id) noexcept = default;
```

比较同类 ID 的身份，不检查实体是否在场，也不区分属于哪张牌桌。
