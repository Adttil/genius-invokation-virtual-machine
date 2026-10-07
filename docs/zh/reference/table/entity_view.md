[givm](../../reference.md) / [牌桌](../table.md) / **entity_view**

# givm::entity_view

定义于头文件 `<givm/table.hpp>`

```cpp
template<entity_category Category>
using entity_view = /* 相应的只读实体视图类型 */;
```

按实体类别取得只读视图类型。例如 `entity_view<entity_category::character>` 是 `character_view`，`entity_view<entity_category::hand_card>` 是 `hand_card_view`。

`Category` 必须是实际实体类别，不能是 `null`。

## 参阅

| | |
| --- | --- |
| [`character_view::category`](character_view/category.md) | 视图自身提供的实体类别，以角色为例 |
| [`entity_category`](../enums/entity_category.md) | 实体形态 |
