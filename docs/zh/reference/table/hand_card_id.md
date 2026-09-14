[givm](../../reference.md) / [牌桌](../table.md) / **hand_card_id**

# givm::hand_card_id

定义于头文件 `<givm/table/entity_id.hpp>`

```cpp
struct hand_card_id;
```

手牌在一张牌桌中的身份。使用此 ID 可以通过 [`card_table::operator[]`](card_table/operator_subscript.md) 再次取得相应实体。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player_id` | [`player_id`](player_id.md) | 所属玩家的 ID |
| `index` | `size_t` | 该实体的标识值；不表示筛除已移除实体后的排列位置 |

## 非成员函数

```cpp
friend constexpr bool operator==(hand_card_id, hand_card_id) = default;
```

比较各成员是否相等；比较不检查实体是否尚未移除。

## 注意

ID 本身不包含牌桌身份。清理实体后，原有 ID 可能失效；详见[实体的身份与访问](entity_access.md)。


## 示例

```cpp
#include <cstdint>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct example_source
{
    using definition_category = givm::card_definition;
    struct definition_type {};
    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    example_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::draw_cards{ .count = 1 }, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{});
    const auto definition = ids.get_id<givm::card_definition>("示例");
    givm::card_table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .cards = { definition } });

    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(library, table, random);
    const givm::hand_card_view view = table[givm::hand_card_id{ givm::player_id{ 0 }, 0 }];
    const givm::hand_card_id id = view.id();
    std::println("通过 ID 取得定义: {}", library[table[id].definition_id()].name());
}
```

输出

```text
通过 ID 取得定义: 示例
```
