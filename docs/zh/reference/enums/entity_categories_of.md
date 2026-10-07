[givm](../../reference.md) / [枚举值](../enums.md) / **entity_categories_of**

# givm::entity_categories_of

定义于头文件 `<givm/enums/entity_category.hpp>`

```cpp
template<definition_category Category>
inline constexpr std::span<const entity_category> entity_categories_of = /* 对应实体类别范围 */;
```

取得采用该类定义的实体形态。返回范围按实体枚举顺序排列，借用静态存储，可用于常量表达式，也可长期保存。

卡牌返回手牌、牌库牌两种类别；卡牌状态返回相应的两种状态类别。历史摘要和 `null` 返回空范围。

## 示例

```cpp
#include <givm/table.hpp>
#include <print>

int main()
{
    constexpr auto cards = givm::entity_categories_of<givm::definition_category::card>;
    static_assert(cards[0] == givm::entity_category::hand_card);
    std::println("卡牌实体形态数量: {}", cards.size());
    std::println("历史摘要实体形态数量: {}",
        givm::entity_categories_of<givm::definition_category::history_summary>.size());
}
```

输出

```text
卡牌实体形态数量: 2
历史摘要实体形态数量: 0
```
