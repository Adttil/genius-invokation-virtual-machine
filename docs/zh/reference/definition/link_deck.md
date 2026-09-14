[givm](../../reference.md) / [定义](../definition.md) / **link_deck**

# givm::link_deck

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TCardNames, class TCharacterNames>
linked_deck link_deck(
    const issued_id_map& id_map,
    TCardNames&& card_names,
    TCharacterNames&& character_names
);
```

把按名称描述的牌组转换为已确定定义的牌组，保留输入的顺序和重复项。

## 模板参数

|  |  |
| --- | --- |
| `TCardNames` | 可遍历的卡牌名称范围，元素可构造 `std::string_view` |
| `TCharacterNames` | 可遍历的角色名称范围，元素可构造 `std::string_view` |

## 参数

|  |  |
| --- | --- |
| `id_map` | 准备使用的定义库所配套的 ID 映射 |
| `card_names` | 卡牌定义名称 |
| `character_names` | 角色定义名称 |

## 返回值

持有对应定义 ID 的 [`linked_deck`](../table/linked_deck.md)。

## 异常

|  |  |
| --- | --- |
| `std::invalid_argument` | 卡牌或角色名称不在映射中 |

## 注意

本函数解析名称，不检查游戏规则对卡牌数量、重复数或角色组合的限制。

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
    const auto deck = givm::link_deck(
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
