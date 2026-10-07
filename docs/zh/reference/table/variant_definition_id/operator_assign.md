[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_definition_id](../variant_definition_id.md) / **operator_assign**

# givm::variant_definition_id::operator_assign

```cpp
constexpr variant_definition_id& operator=(std::uint64_t word);
```

从编码字赋值，使用整数构造的 Debug 检查并返回 `*this`。允许的单类别 ID、较窄类别集合及可空类型的 `nullptr` 也可通过转换后赋值。复制与移动赋值保持平凡。
