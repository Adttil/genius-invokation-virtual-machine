[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_id](../deck_card_id.md) / **operator_assign**

# givm::deck_card_id::operator_assign

```cpp
constexpr deck_card_id& operator=(const deck_card_id&) noexcept = default;
constexpr deck_card_id& operator=(deck_card_id&&) noexcept = default;
```

隐式生成的复制和移动赋值保持平凡，只接受同类 ID。
