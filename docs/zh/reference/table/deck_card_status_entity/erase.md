[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_status_entity](../deck_card_status_entity.md) / **erase**

# givm::deck_card_status_entity::erase

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr void erase() const requires is_mutable;
```

使这个牌库卡牌上的状态离场。

## 返回值

（无）

## 注意

实体必须有效且允许修改。该操作不广播离场事件；需要规则结算时应使用相应指令。移除与清理的区别见[实体的身份与访问](../entity_access.md)。

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
    entity.erase();
    std::println("实体尚未移除: {}", entity.is_valid());
}
```

输出

```text
实体尚未移除: false
```
