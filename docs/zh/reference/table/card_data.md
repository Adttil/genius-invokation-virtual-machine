[givm](../../reference.md) / [牌桌](../table.md) / **card_data**

# givm::card_data

定义于头文件 `<givm/table.hpp>`

```cpp
struct card_data;
```

从一个区域取出并准备移到另一区域的卡牌数据。通过它可以在同一张牌桌内移动卡牌，并保留卡牌附带的状态。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `definition_id` | `definition_id<card_definition>` | 该卡牌的定义 ID |
| `state` | [`card_state`](card_state.md) | 卡牌自身的状态 |

## 注意

通过玩家的 `take_*` 接口取得，再交给 `add_hand_card` 或 [`insert_deck_card`](../executor/instructions/insert_deck_card.md)。`first_status` 和 `last_status` 关联卡牌携带的状态；调用方在搬运中原样保留它们，不应手工修改。

这份数据不独立拥有附带状态，不能跨牌桌搬运，不能在搬运过程中清理牌桌，也不能把同一份附带状态重复装入多个位置。

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>
#include <utility>

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
    const auto entity = player.add_deck_card(definition, {});
    const auto status_definition = id_map.get_id<givm::status_definition>("示例");
    entity.add(status_definition, { .count = 4 });
    givm::card_data card = player.take_top_deck_card();
    const auto hand_card = player.add_hand_card(std::move(card));
    for(const auto status : hand_card.statuses())
    {
        std::println("保留的状态计数: {}", status.state().count);
    }
}
```

输出

```text
保留的状态计数: 4
```
