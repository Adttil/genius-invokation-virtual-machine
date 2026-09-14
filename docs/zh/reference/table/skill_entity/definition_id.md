[givm](../../../reference.md) / [牌桌](../../table.md) / [skill_entity](../skill_entity.md) / **definition_id**

# givm::skill_entity::definition_id

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto definition_id() const;
```

取得该实体采用的定义 ID。

## 返回值

该实体的 [`definition_id`](../definition_id.md)。

## 注意

实体必须有效。

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
    std::println("采用指定定义: {}", entity.definition_id() == definition);
}
```

输出

```text
采用指定定义: true
```
