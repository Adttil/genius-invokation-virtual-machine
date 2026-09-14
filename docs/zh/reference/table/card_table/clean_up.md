[givm](../../../reference.md) / [牌桌](../../table.md) / [card_table](../card_table.md) / **clean_up**

# givm::card_table::clean_up

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr void clean_up() noexcept;
```

清理双方已经移除的实体及卡牌状态，保留尚未移除的实体。

## 返回值

（无）

## 注意

清理可能改变实体 ID，并使已取得的实体访问对象、范围和状态引用失效。清理后应重新从牌桌获取它们；不要在结算仍持有这些对象时调用。牌库中尚未移除卡牌的先后顺序保持不变。

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
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");

    player.add_hand_card(definition, {});
    player.add_hand_card(definition, {});
    table[givm::hand_card_id{ { 0 }, 0 }].erase();
    std::println("清理前全部手牌位置: {}", std::ranges::distance(player.hand_cards<false>()));
    table.clean_up();
    std::println("清理后手牌数量: {}", table[givm::player_id{ 0 }].hand_card_count());
}
```

输出

```text
清理前全部手牌位置: 2
清理后手牌数量: 1
```
