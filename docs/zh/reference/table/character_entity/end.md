[givm](../../../reference.md) / [牌桌](../../table.md) / [character_entity](../character_entity.md) / **end**

# givm::character_entity::end

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto end() const;
```

取得这个角色的单实体范围终点。

## 返回值

实体有效时为单实体范围的尾后指针，否则与 `begin()` 相等。

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
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::character_view>("示例");
    const auto entity = player.add(definition, { .max_health = 10, .health = 10 });
    std::println("移除前范围为空: {}", entity.begin() == entity.end());
    entity.erase();
    std::println("移除后范围为空: {}", entity.begin() == entity.end());
}
```

输出

```text
移除前范围为空: false
移除后范围为空: true
```
