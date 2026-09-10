[givm](../../../reference.md) / [牌桌](../../table.md) / [card_status_range](../card_status_range.md) / **iterator**

# givm::card_status_range::iterator

定义于头文件 `<givm/table.hpp>`

```cpp
class iterator;
```

卡牌状态遍历中的一个位置，用于访问当前状态并前进到下一个状态。

## 成员函数

|  |  |
| --- | --- |
| [`operator*`](iterator/operator_dereference.md) | 取得当前状态 |
| [`operator++`](iterator/operator_increment.md) | 前进到下一个状态 |

## 非成员函数

|  |  |
| --- | --- |
| [`operator==`](iterator/operator_equal.md) | 比较是否处于同一遍历位置 |

## 注意

通过 `card_status_range::begin()` 和 `end()` 取得；支持解引用、前置递增和相等比较，可用于范围 `for`。不提供后置递增或 `operator->`。

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
    const auto statuses = entity.statuses();
    for(const auto status : statuses)
    {
        std::println("状态计数: {}", status.state().count);
    }
}
```

输出

```text
状态计数: 2
状态计数: 3
```
