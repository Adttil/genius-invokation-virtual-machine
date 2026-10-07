[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_entity_id](../variant_entity_id.md) / **constructor**

# givm::variant_entity_id::constructor

```cpp
constexpr variant_entity_id() noexcept;
constexpr variant_entity_id(std::nullptr_t) noexcept; // 仅可空类型
```

可空类型默认保存空值，非空类型默认构造保持平凡。允许的单类别 ID 可隐式转换为该类型，另一个类别集合为本集合子集的多类别 ID 也可隐式转换。
