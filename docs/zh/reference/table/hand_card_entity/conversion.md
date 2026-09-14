[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_entity](../hand_card_entity.md) / **operator hand_card_view**

# givm::hand_card_entity::operator hand_card_view

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr operator hand_card_entity<const detail::table_storage>() const noexcept
    requires is_mutable;
```

将手牌的可写访问对象转换为同一实体的只读视图。

## 返回值

访问同一实体的 [`hand_card_view`](../hand_card_view.md)。

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
    const auto entity = player.add_hand_card(definition, {});
    const givm::hand_card_view view = entity;
    std::println("卡牌在场: {}", view.is_valid());
}
```

输出

```text
卡牌在场: true
```
