[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **deck_card_definition**

# givm::player_entity::deck_card_definition

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr definition_id<card_definition> deck_card_definition(size_t index) const;
```

取得牌库指定位置上的卡牌定义，以便查询该位置是什么牌。

## 参数

|  |  |
| --- | --- |
| `index` | 小于 deck_card_count() 的牌库顺序位置 |

## 返回值

该位置上的 [`definition_id<card_definition>`](../../definition/definition_id.md)。

## 示例

```cpp
#include <print>
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
    player.add_deck_card(definition, {});
    const auto top_definition = player.deck_card_definition(player.deck_card_count() - 1);
    std::println("牌库顶定义: {}", library[top_definition].name());
}
```

输出

```text
牌库顶定义: 示例
```
