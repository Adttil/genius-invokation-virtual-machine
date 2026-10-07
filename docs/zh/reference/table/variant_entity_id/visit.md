[givm](../../../reference.md) / [牌桌](../../table.md) / [variant_entity_id](../variant_entity_id.md) / **visit**

# givm::variant_entity_id::visit

```cpp
template<class F>
constexpr decltype(auto) visit(F&& fn) const;
```

按当前类别把单类别 ID 的值传入访问器；空值传入 `nullptr`。访问器须接受所有允许的类别，并使各调用具有一致的返回类型。不能返回或保存指向所收到临时 ID 的引用。
