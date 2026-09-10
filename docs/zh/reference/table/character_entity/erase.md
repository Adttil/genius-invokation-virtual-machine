[givm](../../../reference.md) / [牌桌](../../table.md) / [character_entity](../character_entity.md) / **erase**

# givm::character_entity::erase

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr void erase() const requires is_mutable;
```

使这个角色离场。其技能和附属实体一并移除。

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
    const example_source<givm::character_view> character_source{};
    sources.add(character_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::character_view>("示例");
    const auto entity = player.add(definition, { .max_health = 10, .health = 10 });
    entity.erase();
    std::println("实体尚未移除: {}", entity.is_valid());
}
```

输出

```text
实体尚未移除: false
```
