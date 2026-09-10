[givm](../../../reference.md) / [牌桌](../../table.md) / [combat_status_entity](../combat_status_entity.md) / **operator combat_status_view**

# givm::combat_status_entity::operator combat_status_view

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr operator combat_status_entity<const detail::table_storage>() const noexcept
    requires is_mutable;
```

将出战状态的可写访问对象转换为同一实体的只读视图。

## 返回值

访问同一实体的 [`combat_status_view`](../combat_status_view.md)。

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
    const example_source<givm::combat_status_view> combat_status_source{};
    sources.add(combat_status_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::combat_status_view>("示例");
    const auto entity = player.add(definition, { .count = 3 });
    const givm::combat_status_view view = entity;
    std::println("计数: {}", view.state().count);
}
```

输出

```text
计数: 3
```
