[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_entity_id](../variant_entity_id.md) / **holds**

# givm::variant_entity_id::holds

```cpp
template<entity_category Category>
constexpr bool holds() const noexcept;

template<class TId>
constexpr bool holds() const noexcept;
```

判断当前类别是否为 `Category`。该类别必须包含在类型允许的类别集合中。

也可使用实际的强 ID 类型作为模板参数，例如 `holds<skill_id>()` 与 `holds<entity_category::skill>()` 等价。允许空值时，`holds<std::nullptr_t>()` 与 `holds<entity_category::null>()` 等价。
