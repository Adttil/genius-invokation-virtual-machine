[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_entity](../hand_card_entity.md) / **is_valid**

# givm::hand_card_entity::is_valid

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr bool is_valid() const;
```

判断这个手牌是否仍然有效。

## 返回值

实体尚未移除时为 `true`，已经移除时为 `false`。

## 注意

访问对象本身必须仍可安全访问；本函数不能用来检查已经悬空的对象。

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
    const example_source<givm::card_definition> card_source{};
    sources.add(card_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    const auto entity = player.add_hand_card(definition, {});
    std::println("移除前有效: {}", entity.is_valid());
    entity.erase();
    std::println("移除后有效: {}", entity.is_valid());
}
```

输出

```text
移除前有效: true
移除后有效: false
```
