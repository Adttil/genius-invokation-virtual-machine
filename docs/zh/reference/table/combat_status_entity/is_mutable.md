[givm](../../../reference.md) / [牌桌](../../table.md) / [combat_status_entity](../combat_status_entity.md) / **is_mutable**

# givm::combat_status_entity::is_mutable

定义于头文件 `<givm/table.hpp>`

```cpp
static constexpr bool is_mutable = not std::is_const_v<TStorage>;
```

该访问对象是否允许修改出战状态。可写实体为 `true`，只读视图为 `false`。

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
    std::println("实体允许修改: {}", decltype(entity)::is_mutable);
    std::println("视图允许修改: {}", decltype(view)::is_mutable);
}
```

输出

```text
实体允许修改: true
视图允许修改: false
```
