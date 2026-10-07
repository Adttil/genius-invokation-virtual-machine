[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_definition_id](../variant_definition_id.md) / **get**

# givm::variant_definition_id::get

```cpp
template<definition_category Category>
constexpr definition_id<Category> get() const;

template<class TId>
constexpr TId get() const;
```

返回指定实际类别的非空 ID。前提是当前类别相符；Debug 违反前提时抛出 `std::invalid_argument`。

只有一种实际类别时可以省略模板参数使用 `get()`；该类型若还允许空值，也可使用 `operator*()`。这些访问都会返回 ID 的值。

也可使用实际的强 ID 类型作为模板参数，例如 `get<skill_definition_id>()` 与 `get<definition_category::skill>()` 等价。
