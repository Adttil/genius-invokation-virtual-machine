[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_entity](../hand_card_entity.md) / **statuses**

# givm::hand_card_entity::statuses

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto statuses() const;
```

遍历附着在这张卡牌上的状态，按添加顺序取得尚未移除的状态。

## 返回值

产生 [`hand_card_status_entity`](../hand_card_status_entity.md) 的 [`card_status_range`](../card_status_range.md)。

## 注意

卡牌必须有效。移除或增加状态后应重新取得范围；不要继续使用受修改影响的遍历位置。

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
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    const auto entity = player.add_hand_card(definition, {});
    const auto status_definition = id_map.get_id<givm::status_definition>("示例");
    entity.add(status_definition, { .count = 2 });
    entity.add(status_definition, { .count = 3 });
    for(const auto status : entity.statuses())
    {
        std::println("附带状态计数: {}", status.state().count);
    }
}
```

输出

```text
附带状态计数: 2
附带状态计数: 3
```
