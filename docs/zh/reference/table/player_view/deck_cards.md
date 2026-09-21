[givm](../../../reference.md) / [牌桌](../../table.md) / [player_view](../player_view.md) / **deck_cards**

# givm::player_view::deck_cards

定义于头文件 `<givm/table.hpp>`

```cpp
template<bool SkipErased = true>
constexpr auto deck_cards() const;
```

遍历该玩家的牌库卡牌。

## 模板参数

|  |  |
| --- | --- |
| `SkipErased` | 是否跳过已经移除的实体，默认为 true |

## 返回值

产生 [`deck_card_view`](../deck_card_view.md) 的范围。

## 注意

遍历所得访问对象的存活约定见[实体的身份与访问](../entity_access.md)。按牌库底到牌库顶的顺序遍历；已移出的牌不会因 `SkipErased = false` 重新出现。


## 示例

```cpp
#include <cstdint>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct example_source
{
    using definition_category = givm::card_definition;
    struct definition_type {};
    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    example_source source{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    const auto definition = ids.get_id<givm::card_definition>("示例");
    givm::table table{};
    load_deck(table, library, givm::linked_deck{ .cards = { definition } }, {});
    const givm::deck_card_view view = table[givm::deck_card_id{ givm::player_id{ 0 }, 0 }];
    for(const auto card : view.player().deck_cards())
    {
        std::println("牌库中的定义: {}", library[card.definition_id()].name());
    }
}
```

输出

```text
牌库中的定义: 示例
```
