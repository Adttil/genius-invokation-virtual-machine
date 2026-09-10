[givm](../../reference.md) / [指令](../instructions.md) / **shuffle_deck**

# givm::shuffle_deck

定义于头文件 `<givm/executor/instructions/shuffle_deck.hpp>`

```cpp
struct shuffle_deck;
```

随机重排指定玩家牌堆中的牌。牌的内容和牌堆数量保持不变。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，可用于不限定事件的操作 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../table/player_id.md) | 要洗牌的玩家 |

## 成员函数

| | |
| --- | --- |
| [`execute`](shuffle_deck/execute.md) | 随机重排指定玩家牌堆中的牌 |

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;
    struct definition_type {};
    std::string_view source_name;
    std::string_view name() const { return source_name; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    card_source first{ "first" };
    card_source second{ "second" };
    givm::definition_source_library sources{};
    sources.add(first, second);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    const auto a = ids.get_id<givm::card_definition>("first");
    const auto b = ids.get_id<givm::card_definition>("second");
    auto player = table[givm::player_id{ 0 }];
    player.insert_deck_card(0, a, {});
    player.insert_deck_card(1, b, {});
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("洗牌后牌数: {}", player.deck_card_count());
    std::println("原底牌变为顶牌: {}", player.deck_card_definition(1).value() == a.value());
}
```

输出

```text
洗牌后牌数: 2
原底牌变为顶牌: true
```
