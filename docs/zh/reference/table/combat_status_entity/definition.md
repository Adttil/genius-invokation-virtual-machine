[givm](../../../reference.md) / [牌桌](../../table.md) / [combat_status_entity](../combat_status_entity.md) / **definition**

# givm::combat_status_entity::definition

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto definition() const;
```

取得说明这个出战状态种类和行为的定义。

## 返回值

配套定义库中该实体定义的 [`definition_view`](../../definition/definition_library/definition_view.md)。

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
    const example_source<givm::combat_status_view> combat_status_source{};
    sources.add(combat_status_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::combat_status_view>("示例");
    const auto entity = player.add(definition, { .count = 3 });
    std::println("定义名称: {}", entity.definition().name());
}
```

输出

```text
定义名称: 示例
```
