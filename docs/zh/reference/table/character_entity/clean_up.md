[givm](../../../reference.md) / [牌桌](../../table.md) / [character_entity](../character_entity.md) / **clean_up**

# givm::character_entity::clean_up

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr void clean_up() const requires is_mutable;
```

清理该角色已经移除的技能和附属实体。

## 返回值

（无）

## 注意

角色必须有效且允许修改。清理后须重新取得其技能和附属实体的 ID、访问对象及范围。

## 示例

```cpp
#include <print>
#include <ranges>
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
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto character_definition = id_map.get_id<givm::character_view>("示例");
    const auto definition = id_map.get_id<givm::skill_view>("示例");
    const auto character = player.add(character_definition, { .max_health = 10, .health = 10 });
    const auto entity = character.add(definition, { .count = 3 });
    entity.erase();
    character.clean_up();
    std::println("清理后全部技能位置: {}", std::ranges::distance(character.skills<false>()));
}
```

输出

```text
清理后全部技能位置: 0
```
