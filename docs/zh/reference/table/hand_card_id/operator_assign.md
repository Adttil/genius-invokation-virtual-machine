[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_id](../hand_card_id.md) / **operator_assign**

# givm::hand_card_id::operator_assign

```cpp
constexpr hand_card_id& operator=(const hand_card_id&) noexcept = default;
constexpr hand_card_id& operator=(hand_card_id&&) noexcept = default;
```

隐式生成的复制和移动赋值保持平凡，只接受同类 ID。
