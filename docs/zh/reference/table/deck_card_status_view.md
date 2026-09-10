[givm](../../reference.md) / [牌桌](../table.md) / **deck_card_status_view**

# givm::deck_card_status_view

定义于头文件 `<givm/definition/types.hpp>`

```cpp
using deck_card_status_view = deck_card_status_entity<const detail::table_storage>;
```

牌库卡牌上的状态的只读视图。它可以查看实体的状态和所属关系，不能修改该实体。

该视图仍然访问原牌桌中的实体；创建视图不会冻结或复制对局状态。

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
    const givm::deck_card_status_view view = entity;
    std::println("状态计数: {}", view.state().count);
}
```

输出

```text
状态计数: 3
```

## 参阅

|  |  |
| --- | --- |
| [`deck_card_status_entity`](deck_card_status_entity.md) | 实体的完整访问接口 |
| [实体的身份与访问](entity_access.md) | 只读访问与存活约定 |
