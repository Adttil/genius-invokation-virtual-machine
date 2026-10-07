[givm](../../../reference.md) / [牌桌](../../table.md) / [player_id](../player_id.md) / **operator_assign**

# givm::player_id::operator_assign

```cpp
constexpr player_id& operator=(const player_id&) noexcept = default;
constexpr player_id& operator=(player_id&&) noexcept = default;
```

隐式生成的复制和移动赋值保持平凡，只接受同类 ID。
