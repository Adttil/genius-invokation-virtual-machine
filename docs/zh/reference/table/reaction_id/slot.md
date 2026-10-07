[givm](../../../reference.md) / [牌桌](../../table.md) / [reaction_id](../reaction_id.md) / **slot**

# givm::reaction_id::slot

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr elemental_reaction slot() const noexcept;
```

取得非空反应槽位。没有反应时使用 `optional_reaction_id` 的空值。
