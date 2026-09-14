[givm](../../../reference.md) / [牌桌](../../table.md) / [skill_entity](../skill_entity.md) / **is_mutable**

# givm::skill_entity::is_mutable

定义于头文件 `<givm/table.hpp>`

```cpp
static constexpr bool is_mutable = not std::is_const_v<TStorage>;
```

该访问对象是否允许修改技能。可写实体为 `true`，只读视图为 `false`。

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
    const example_source<givm::skill_view> skill_source{};
    sources.add(character_source, skill_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto character_definition = id_map.get_id<givm::character_view>("示例");
    const auto definition = id_map.get_id<givm::skill_view>("示例");
    const auto character = player.add(character_definition, { .max_health = 10, .health = 10 });
    const auto entity = character.add(definition, { .count = 3 });
    const givm::skill_view view = entity;
    std::println("实体允许修改: {}", decltype(entity)::is_mutable);
    std::println("视图允许修改: {}", decltype(view)::is_mutable);
}
```

输出

```text
实体允许修改: true
视图允许修改: false
```
