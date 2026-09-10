[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_status_entity](../deck_card_status_entity.md) / **id**

# givm::deck_card_status_entity::id

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr deck_card_status_id id() const;
```

取得牌库卡牌上的状态的身份，以便之后从牌桌再次访问它。

## 返回值

该实体的 [`deck_card_status_id`](../deck_card_status_id.md)。

## 注意

实体必须有效。ID 的保存期限见[实体的身份与访问](../entity_access.md)。

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
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto card_definition = id_map.get_id<givm::card_definition>("示例");
    const auto definition = id_map.get_id<givm::status_definition>("示例");
    const auto card = player.add_deck_card(card_definition, {});
    const auto entity = card.add(definition, { .count = 3 });
    const auto id = entity.id();
    std::println("再次找到同一实体: {}", table[id].id() == id);
}
```

输出

```text
再次找到同一实体: true
```
