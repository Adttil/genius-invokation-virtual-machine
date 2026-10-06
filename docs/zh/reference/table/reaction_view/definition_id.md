[givm](../../../reference.md) / [牌桌](../../table.md) / [reaction_view](../reaction_view.md) / **definition_id**

# givm::reaction_view::definition_id

定义于头文件 `<givm/table.hpp>`。

```cpp
constexpr definition_id<reaction_view> definition_id() const noexcept;
```

读取装载牌组时确定的反应定义 ID。角色死亡不撤销此映射；槽位不能为 none。
