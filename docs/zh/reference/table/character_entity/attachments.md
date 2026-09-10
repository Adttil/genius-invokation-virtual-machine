[givm](../../../reference.md) / [牌桌](../../table.md) / [character_entity](../character_entity.md) / **attachments**

# givm::character_entity::attachments

定义于头文件 `<givm/table.hpp>`

```cpp
template<bool SkipErased = true>
constexpr auto attachments() const;
```

遍历该角色的角色附属实体。

## 模板参数

|  |  |
| --- | --- |
| `SkipErased` | 是否跳过已经移除的实体，默认为 true |

## 返回值

产生 [`attachment_entity`](../attachment_entity.md) 的范围。

## 注意

角色必须有效。范围中的实体具有与角色访问对象相同的读写权限。

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
    for(const auto item : character.attachments())
    {
        std::println("角色附属实体计数: {}", item.state().count);
    }
}
```

输出

```text
角色附属实体计数: 3
```
