[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **hand_cards**

# givm::player_entity::hand_cards

定义于头文件 `<givm/table.hpp>`

```cpp
template<bool SkipErased = true>
constexpr auto hand_cards() const;
```

遍历该玩家的手牌。

## 模板参数

|  |  |
| --- | --- |
| `SkipErased` | 是否跳过已经移除的实体，默认为 true |

## 返回值

产生 [`hand_card_entity`](../hand_card_entity.md) 的范围；实体具有与玩家访问对象相同的读写权限。

## 注意

遍历所得访问对象的存活约定见[实体的身份与访问](../entity_access.md)。

## 示例

```cpp
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

template<class Category>
struct example_source
{
    using definition_category = Category;
    struct definition_type {};

    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    givm::definition_source_library sources{};
    const example_source<givm::card_definition> card_source{};
    sources.add(card_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    const auto entity = player.add_hand_card(definition, {});
    std::println("移除前数量: {}", std::ranges::distance(player.hand_cards()));
    entity.erase();
    std::println("移除后数量: {}", std::ranges::distance(player.hand_cards()));
}
```

输出

```text
移除前数量: 1
移除后数量: 0
```
