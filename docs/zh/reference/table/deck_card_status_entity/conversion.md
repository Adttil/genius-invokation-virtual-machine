[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_status_entity](../deck_card_status_entity.md) / **operator deck_card_status_view**

# givm::deck_card_status_entity::operator deck_card_status_view

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr operator deck_card_status_entity<const detail::table_storage>() const noexcept
    requires is_mutable;
```

将牌库卡牌上的状态的可写访问对象转换为同一实体的只读视图。

## 返回值

访问同一实体的 [`deck_card_status_view`](../deck_card_status_view.md)。

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
    const example_source<givm::status_definition> status_source{};
    sources.add(card_source, status_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto card_definition = id_map.get_id<givm::card_definition>("示例");
    const auto definition = id_map.get_id<givm::status_definition>("示例");
    const auto card = player.add_deck_card(card_definition, {});
    const auto entity = card.add(definition, { .count = 3 });
    const givm::deck_card_status_view view = entity;
    std::println("状态计数: {}", view.state().count);
}
```

输出

```text
状态计数: 3
```
