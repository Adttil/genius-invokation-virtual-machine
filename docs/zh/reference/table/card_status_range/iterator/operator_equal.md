[givm](../../../../reference.md) / [牌桌](../../../table.md) / [card_status_range](../../card_status_range.md) / [iterator](../iterator.md) / **operator==**

# givm::card_status_range::iterator::operator==

定义于头文件 `<givm/table.hpp>`

```cpp
friend constexpr bool operator==(const iterator&, const iterator&) = default;
```

比较两个卡牌状态遍历位置是否相同。

## 参数

|  |  |
| --- | --- |
| 两个迭代器 | 要比较的卡牌状态位置 |

## 返回值

牌桌、所属卡牌和遍历位置均相同时为 `true`，否则为 `false`。

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
    const auto entity = player.add_hand_card(definition, {});
    const auto status_definition = id_map.get_id<givm::status_definition>("示例");
    entity.add(status_definition, { .count = 2 });
    entity.add(status_definition, { .count = 3 });
    const auto statuses = entity.statuses();
    const auto first = statuses.begin();
    auto second = first;
    std::println("复制后位置相同: {}", first == second);
    ++second;
    std::println("前进后位置相同: {}", first == second);
}
```

输出

```text
复制后位置相同: true
前进后位置相同: false
```
