[givm](../../../reference.md) / [牌桌](../../table.md) / [skill_entity](../skill_entity.md) / **can_handle**

# givm::skill_entity::can_handle

定义于头文件 `<givm/table.hpp>`

```cpp
template<class TEvent>
bool can_handle() const;
```

判断该技能是否具有处理指定事件的定义。

## 模板参数

|  |  |
| --- | --- |
| `TEvent` | 该实体视图允许订阅的事件类型，见 [`subscribed_events`](../../definition/subscribed_events.md) |

## 返回值

实体有效且定义包含对应事件的处理程序时为 `true`，否则为 `false`。

## 注意

这里只查询是否具有处理程序，不调用它，也不判断本次事件的具体条件是否会产生效果。

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
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto character_definition = id_map.get_id<givm::character_view>("示例");
    const auto definition = id_map.get_id<givm::skill_view>("示例");
    const auto character = player.add(character_definition, { .max_health = 10, .health = 10 });
    const auto entity = character.add(definition, { .count = 3 });
    std::println("响应行动前事件: {}", entity.can_handle<givm::before_action>());
}
```

输出

```text
响应行动前事件: false
```
