[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_entity](../deck_card_entity.md) / **begin**

# givm::deck_card_entity::begin

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto begin() const;
```

取得这个牌库卡牌的单实体范围起点。

## 返回值

指向该 [`deck_card_entity`](../deck_card_entity.md) 访问对象自身的只读指针。若实体无效，它等于 `end()`。

## 注意

范围依赖访问对象本身的存活。遍历有效实体时执行一次循环；遍历已移除实体时不执行循环。

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
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    const auto entity = player.add_deck_card(definition, {});
    for(const auto& item : entity)
    {
        std::println("范围中的实体: {}", item.id() == entity.id());
    }
}
```

输出

```text
范围中的实体: true
```
