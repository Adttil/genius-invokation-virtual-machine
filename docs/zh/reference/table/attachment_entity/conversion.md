[givm](../../../reference.md) / [牌桌](../../table.md) / [attachment_entity](../attachment_entity.md) / **operator attachment_view**

# givm::attachment_entity::operator attachment_view

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr operator attachment_entity<const detail::table_storage>() const noexcept
    requires is_mutable;
```

将角色附属实体的可写访问对象转换为同一实体的只读视图。

## 返回值

访问同一实体的 [`attachment_view`](../attachment_view.md)。

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
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto character_definition = id_map.get_id<givm::character_view>("示例");
    const auto definition = id_map.get_id<givm::attachment_view>("示例");
    const auto character = player.add(character_definition, { .max_health = 10, .health = 10 });
    const auto entity = character.add(definition, { .count = 3 });
    const givm::attachment_view view = entity;
    std::println("计数: {}", view.state().count);
}
```

输出

```text
计数: 3
```
