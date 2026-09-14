[givm](../../../reference.md) / [牌桌](../../table.md) / [combat_status_entity](../combat_status_entity.md) / **erase**

# givm::combat_status_entity::erase

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr void erase() const requires is_mutable;
```

使这个出战状态离场。

## 返回值

（无）

## 注意

实体必须有效且允许修改。该操作不广播离场事件；需要规则结算时应使用相应指令。移除与清理的区别见[实体的身份与访问](../entity_access.md)。

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
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::combat_status_view>("示例");
    const auto entity = player.add(definition, { .count = 3 });
    entity.erase();
    std::println("实体尚未移除: {}", entity.is_valid());
}
```

输出

```text
实体尚未移除: false
```
