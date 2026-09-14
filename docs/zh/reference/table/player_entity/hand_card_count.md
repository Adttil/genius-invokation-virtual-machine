[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **hand_card_count**

# givm::player_entity::hand_card_count

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr size_t hand_card_count() const noexcept;
```

取得该玩家当前的手牌张数。

## 返回值

仍在手牌中的有效卡牌数量。

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
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    player.add_hand_card(definition, {});
    player.add_hand_card(definition, {});
    std::println("手牌张数: {}", player.hand_card_count());
}
```

输出

```text
手牌张数: 2
```
