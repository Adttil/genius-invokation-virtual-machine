[givm](../../reference.md) / [牌桌](../table.md) / **card_status_range**

# givm::card_status_range

定义于头文件 `<givm/table.hpp>`

```cpp
template<class TStorage, class TStatusEntity, class TOwnerId>
class card_status_range;
```

一张卡牌所携带的状态序列，可依次访问尚未移除的状态。

## 模板参数

|  |  |
| --- | --- |
| `TStorage` | 由牌桌决定的读写类型 |
| `TStatusEntity` | 手牌状态或牌库卡牌状态的实体访问类型 |
| `TOwnerId` | 所属卡牌的 ID 类型 |

## 成员类型

|  |  |
| --- | --- |
| [`iterator`](card_status_range/iterator.md) | 遍历卡牌状态的位置 |

## 成员函数

|  |  |
| --- | --- |
| [`begin`](card_status_range/begin.md) | 取得第一个状态的位置 |
| [`end`](card_status_range/end.md) | 取得遍历终点 |

## 注意

通过卡牌的 `statuses()` 取得范围，无需显式指定模板参数。范围不拥有卡牌或状态；增加、移除状态或清理牌桌后，应重新取得范围。

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
