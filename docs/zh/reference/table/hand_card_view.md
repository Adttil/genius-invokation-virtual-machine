[givm](../../reference.md) / [牌桌](../table.md) / **hand_card_view**

# givm::hand_card_view

定义于头文件 `<givm/table.hpp>`

```cpp
class hand_card_view;
```

玩家手中一张卡牌的只读视图。它代表这张正在参与对局的牌，可以拥有自己的附带状态。

## 成员函数

|  |  |
| --- | --- |
| [`is_valid`](hand_card_view/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](hand_card_view/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](hand_card_view/size.md) | 取得单实体范围的元素数 |
| [`begin`](hand_card_view/begin.md) | 取得单实体范围的起点 |
| [`end`](hand_card_view/end.md) | 取得单实体范围的终点 |
| [`player`](hand_card_view/player.md) | 取得所属玩家 |
| [`id`](hand_card_view/id.md) | 取得实体 ID |
| [`definition_id`](hand_card_view/definition_id.md) | 取得实体的定义 ID |
| [`state`](hand_card_view/state.md) | 访问实体状态 |
| [`statuses`](hand_card_view/statuses.md) | 遍历卡牌状态 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个手牌。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。


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
    std::println("采用已加载的定义: {}", view.definition_id() == definition);
}
```

输出

```text
采用已加载的定义: true
```

## 参阅

|  |  |
| --- | --- |
| [`card_state`](card_state.md) | 该实体的状态 |
