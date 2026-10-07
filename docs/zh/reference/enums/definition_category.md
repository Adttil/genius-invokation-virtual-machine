[givm](../../reference.md) / [枚举值](../enums.md) / **definition_category**

# givm::definition_category

定义于头文件 `<givm/enums/definition_category.hpp>`

```cpp
enum class definition_category : std::uint8_t
{
    card, card_status, support, summon, combat_status,
    character, skill, attachment, history_summary, reaction, null
};
```

卡牌、角色和持续效果采用的规则类别。手牌与牌库牌共用卡牌定义，两者的卡牌状态也共用状态定义；历史摘要有自己的定义类别，其字段直接存放在牌桌上。

## 枚举值

| 名称 | 说明 |
| --- | --- |
| `card` | 卡牌定义 |
| `card_status` | 卡牌状态定义 |
| `support` | 支援定义 |
| `summon` | 召唤物定义 |
| `combat_status` | 出战状态定义 |
| `character` | 角色定义 |
| `skill` | 技能定义 |
| `attachment` | 角色附属实体定义 |
| `history_summary` | 历史摘要定义 |
| `reaction` | 反应定义 |
| `null` | 没有定义；位于最后，其值也是实际定义类别的数量 |

## 注意

定义源声明 `static constexpr auto category = givm::definition_category::card;` 指定类别。定义 ID、源库和编译上下文的类别模板参数均使用该枚举。

类别与实体形态的关系见 [`definition_category_of`](definition_category_of.md) 和 [`entity_categories_of`](entity_categories_of.md)。
