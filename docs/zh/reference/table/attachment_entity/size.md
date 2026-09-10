[givm](../../../reference.md) / [牌桌](../../table.md) / [attachment_entity](../attachment_entity.md) / **size**

# givm::attachment_entity::size

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr size_t size() const;
```

将这个角色附属实体作为单实体范围时，取得其中的元素数。

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
    const example_source<givm::character_view> character_source{};
    const example_source<givm::attachment_view> attachment_source{};
    sources.add(character_source, attachment_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto character_definition = id_map.get_id<givm::character_view>("示例");
    const auto definition = id_map.get_id<givm::attachment_view>("示例");
    const auto character = player.add(character_definition, { .max_health = 10, .health = 10 });
    const auto entity = character.add(definition, { .count = 3 });
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
