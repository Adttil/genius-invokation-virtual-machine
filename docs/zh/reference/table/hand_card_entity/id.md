[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_entity](../hand_card_entity.md) / **id**

# givm::hand_card_entity::id

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr hand_card_id id() const;
```

取得手牌的身份，以便之后从牌桌再次访问它。

## 返回值

该实体的 [`hand_card_id`](../hand_card_id.md)。

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
    sources.add(card_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    const auto entity = player.add_hand_card(definition, {});
    const auto id = entity.id();
    std::println("再次找到同一实体: {}", table[id].id() == id);
}
```

输出

```text
再次找到同一实体: true
```
