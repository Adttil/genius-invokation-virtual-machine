[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_entity](../deck_card_entity.md) / **add**

# givm::deck_card_entity::add

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr deck_card_status_entity<TStorage> add(
    definition_id<status_definition> definition_id, const status_state& state
) const requires is_mutable;
```

在这张卡牌上附加一个状态。

## 参数

|  |  |
| --- | --- |
| [`definition_id`](../definition_id.md) | 配套定义库中的卡牌状态定义 ID |
| `state` | 状态的初始计数 |

## 返回值

新增加的 [`deck_card_status_entity`](../deck_card_status_entity.md)。

## 注意

卡牌必须有效且允许修改。增加状态不自动合并同类状态，也不广播创建事件。

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
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    const auto entity = player.add_deck_card(definition, {});
    const auto status_definition = id_map.get_id<givm::status_definition>("示例");
    const auto status = entity.add(status_definition, { .count = 2 });
    std::println("状态计数: {}", status.state().count);
    std::println("附着在此卡牌: {}", status.card().id() == entity.id());
}
```

输出

```text
状态计数: 2
附着在此卡牌: true
```
