[givm](../../reference.md) / [枚举值](../enums.md) / **definition_category_of**

# givm::definition_category_of

定义于头文件 `<givm/enums/entity_category.hpp>`

```cpp
template<entity_category Category>
inline constexpr definition_category definition_category_of = /* 对应定义类别 */;
```

取得实体类别采用的定义类别。手牌和牌库牌均为 `card`，两种卡牌状态均为 `card_status`；玩家和 `null` 没有定义，结果为 `null`。

实体视图通过自己的静态成员 `category` 提供实体类别。

## 示例

```cpp
#include <givm/table.hpp>
#include <print>
int main()
{
    constexpr auto category = givm::definition_category_of<givm::deck_card_view::category>;
    std::println("牌库牌采用卡牌定义: {}", category == givm::definition_category::card);
}
```

输出

```text
牌库牌采用卡牌定义: true
```
