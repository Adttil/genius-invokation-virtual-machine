[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_status_entity](../hand_card_status_entity.md) / **state**

# givm::hand_card_status_entity::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto& state() const;
```

访问该手牌上的状态在对局中的当前状态。

## 返回值

实体持有的 [`status_state`](../status_state.md) 引用；可写实体返回可写引用，只读视图返回只读引用。

## 注意

实体必须有效。直接修改状态不执行规则结算或事件广播。

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
    const example_source<givm::status_definition> status_source{};
    sources.add(card_source, status_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto card_definition = id_map.get_id<givm::card_definition>("示例");
    const auto definition = id_map.get_id<givm::status_definition>("示例");
    const auto card = player.add_hand_card(card_definition, {});
    const auto entity = card.add(definition, { .count = 3 });
    entity.state().count = 2;
    std::println("状态计数: {}", entity.state().count);
}
```

输出

```text
状态计数: 2
```
