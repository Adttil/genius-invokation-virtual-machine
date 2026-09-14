[givm](../../reference.md) / [牌桌](../table.md) / **linked_deck**

# givm::linked_deck

定义于头文件 `<givm/table.hpp>`

```cpp
struct linked_deck;
```

已经确定卡牌与角色定义的牌组。它保留牌组配置中的卡牌顺序、角色顺序和重复卡牌，可交给牌桌载入。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cards` | `std::vector<definition_id<card_definition>>` | 卡牌定义序列 |
| `characters` | `std::vector<definition_id<character_view>>` | 角色定义序列 |

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{};
    ids.add<givm::card_definition>("恢复药剂", {});
    ids.add<givm::character_view>("测试角色", {});
    const auto deck = link_deck(
        ids,
        std::array<std::string_view, 2>{ "恢复药剂", "恢复药剂" },
        std::array<std::string_view, 1>{ "测试角色" }
    );
    std::println("卡牌数量: {}", deck.cards.size());
    std::println("角色数量: {}", deck.characters.size());
    std::println("两张牌采用同一定义: {}", deck.cards[0].value() == deck.cards[1].value());
}
```

输出

```text
卡牌数量: 2
角色数量: 1
两张牌采用同一定义: true
```

## 参阅

|  |  |
| --- | --- |
| [`link_deck`](../definition/link_deck.md) | 按名称准备牌组 |
| [`table::load_deck`](table/load_deck.md) | 将牌组载入牌桌 |
