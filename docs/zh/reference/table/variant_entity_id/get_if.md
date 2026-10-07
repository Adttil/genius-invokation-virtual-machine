[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_entity_id](../variant_entity_id.md) / **get_if**

# givm::variant_entity_id::get_if

```cpp
template<entity_category Category>
constexpr optional_entity_id<Category> get_if() const noexcept;
```

类别相符则返回相应 ID，否则返回空值。返回对象仍只占一个 64 位字；提取其非空值可使用 `get()` 或 `operator*()`。
