[givm](../../../reference.md) / [牌桌](../../table.md) / [reaction_view](../reaction_view.md) / **player**

# givm::reaction_view::player

定义于头文件 `<givm/table.hpp>`。

```cpp
constexpr player_view player() const;
```

取得该反应映射所属玩家的只读视图。该玩家可能不同于实际引发反应的来源玩家。
