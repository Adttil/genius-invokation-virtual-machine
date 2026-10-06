[givm](../../../reference.md) / [定义](../../definition.md) / [reaction_definition_names](../reaction_definition_names.md) / **构造函数**

# givm::reaction_definition_names::reaction_definition_names

```cpp
constexpr reaction_definition_names() noexcept;
constexpr explicit reaction_definition_names(std::string_view name) noexcept;
```

默认构造令所有槽位名称为空。带名称构造将 17 个槽位都设为同一名称，适合用同一个定义处理多个基础反应。

保存的是 string_view，名称字符串需要保持有效直到编译结束。
