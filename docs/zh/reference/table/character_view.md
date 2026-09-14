[givm](../../reference.md) / [牌桌](../table.md) / **character_view**

# givm::character_view

定义于头文件 `<givm/table.hpp>`

```cpp
using character_view = character_entity<const detail::table_storage>;
```

角色的只读视图。它可以查看实体的状态和所属关系，不能修改该实体。

该视图仍然访问原牌桌中的实体；创建视图不会冻结或复制对局状态。

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
    const givm::character_view view = entity;
    std::println("生命值: {}", view.state().health);
}
```

输出

```text
生命值: 10
```

## 参阅

|  |  |
| --- | --- |
| [`character_entity`](character_entity.md) | 实体的完整访问接口 |
| [实体的身份与访问](entity_access.md) | 只读访问与存活约定 |
