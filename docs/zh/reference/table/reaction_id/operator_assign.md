[givm](../../../reference.md) / [牌桌](../../table.md) / [reaction_id](../reaction_id.md) / **operator_assign**

# givm::reaction_id::operator_assign

```cpp
constexpr reaction_id& operator=(const reaction_id&) noexcept = default;
constexpr reaction_id& operator=(reaction_id&&) noexcept = default;
```

隐式生成的复制和移动赋值保持平凡，只接受同类 ID。
