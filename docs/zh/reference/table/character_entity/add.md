[givm](../../../reference.md) / [牌桌](../../table.md) / [character_entity](../character_entity.md) / **add**

# givm::character_entity::add

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr skill_entity<TStorage> add(
    definition_id<skill_view> definition_id, const skill_state& state
) const requires is_mutable;

constexpr attachment_entity<TStorage> add(
    definition_id<attachment_view> definition_id, const attachment_state& state
) const requires is_mutable;
```

为角色加入一个技能或附属实体。

## 参数

|  |  |
| --- | --- |
| [`definition_id`](../definition_id.md) | 配套定义库中的技能或附属实体定义 ID |
| `state` | 新实体的初始状态 |

## 返回值

新加入的 [`skill_entity`](../skill_entity.md) 或 [`attachment_entity`](../attachment_entity.md)。

## 注意

角色必须有效且允许修改。本函数不广播创建事件。

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
    std::println("技能计数: {}", entity.state().count);
    std::println("所属角色生命值: {}", entity.character().state().health);
}
```

输出

```text
技能计数: 3
所属角色生命值: 10
```
