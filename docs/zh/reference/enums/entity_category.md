[givm](../../reference.md) / [枚举值](../enums.md) / **entity_category**

# givm::entity_category

定义于头文件 `<givm/enums/entity_category.hpp>`

```cpp
enum class entity_category : std::uint8_t
{
    player, hand_card, deck_card, hand_card_status, deck_card_status,
    support, summon, combat_status, character, skill, attachment, reaction, null
};
```

玩家及牌桌上各类实体的身份类别。一个类别对应一种只读实体视图；同一类定义可以被不同实体形态采用。

## 枚举值

除 `null` 外，各枚举值分别对应同名的 `*_view`。`null` 表示没有实体，位于末尾，其值也是实际实体类别的数量。

历史摘要没有实体类别。它通过历史字段键访问标量引用或数组 span。

## 参阅

| | |
| --- | --- |
| [`entity_view`](../table/entity_view.md) | 从实体类别取得只读 view 类型 |
| [`definition_category_of`](definition_category_of.md) | 实体类别对应的定义类别 |
| [`entity_categories_of`](entity_categories_of.md) | 从定义类别取得实体类别范围 |
