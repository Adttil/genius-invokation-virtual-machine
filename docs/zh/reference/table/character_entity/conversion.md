[givm](../../../reference.md) / [牌桌](../../table.md) / [character_entity](../character_entity.md) / **operator character_view**

# givm::character_entity::operator character_view

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr operator character_entity<const detail::table_storage>() const noexcept
    requires is_mutable;
```

将角色的可写访问对象转换为同一实体的只读视图。

## 返回值

访问同一实体的 [`character_view`](../character_view.md)。

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
    sources.add(character_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::character_view>("示例");
    const auto entity = player.add(definition, { .max_health = 10, .health = 10 });
    const givm::character_view view = entity;
    std::println("生命值: {}", view.state().health);
}
```

输出

```text
生命值: 10
```
