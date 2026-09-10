[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_entity](../hand_card_entity.md) / **size**

# givm::hand_card_entity::size

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr size_t size() const;
```

将这个手牌作为单实体范围时，取得其中的元素数。

## 返回值

实体有效时返回 1，否则返回 0。

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
    std::println("移除前元素数: {}", entity.size());
    entity.erase();
    std::println("移除后元素数: {}", entity.size());
}
```

输出

```text
移除前元素数: 1
移除后元素数: 0
```
