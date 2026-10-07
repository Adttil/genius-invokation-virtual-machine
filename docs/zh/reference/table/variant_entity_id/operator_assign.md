[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_entity_id](../variant_entity_id.md) / **operator_assign**

# givm::variant_entity_id::operator_assign

```cpp
constexpr variant_entity_id& operator=(const variant_entity_id&) noexcept = default;
constexpr variant_entity_id& operator=(variant_entity_id&&) noexcept = default;
```

复制与移动赋值保持平凡。允许的单类别 ID、较窄类别集合及可空类型的 `nullptr` 也可通过转换后赋值。
