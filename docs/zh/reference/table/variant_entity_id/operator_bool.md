[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_entity_id](../variant_entity_id.md) / **operator_bool**

# givm::variant_entity_id::operator_bool

```cpp
constexpr explicit operator bool() const noexcept;
constexpr bool has_value() const noexcept;
```

仅可空类型提供。当前类别不是 `null` 时返回 true；不检查定义或实体是否存在、是否已删除。
