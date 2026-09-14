[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **shuffle_deck**

# givm::shuffle_deck

定义于头文件 `<givm/executor.hpp>`

```cpp
struct shuffle_deck;
```

洗牌指令，用于随机重排指定玩家的牌堆。牌的内容和牌堆数量保持不变。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 要洗牌的玩家 |

## 注意

对 `N` 张牌调用随机源 `max(N - 1, 0)` 次。按牌序从底至顶编号为 `0` 至 `N - 1`，洗牌结果由以下规则确定：

1. 依次令 `m` 为 `N`、`N - 1`，直到 `2`。
2. 每次取得一个 `std::uint32_t` 随机值 `r`，令 `j = floor(r × m / 2^32)`。
3. 交换当前位置 `m - 1` 与位置 `j` 的牌。

这里的乘除按数学整数计算，`r` 的范围为 `0` 至 `2^32 - 1`。每次 `j` 都在 `0` 至 `m - 1` 内；即使两位置相同，也消耗该随机值。空牌堆和单张牌堆不调用随机源。例如两张牌由底至顶为 `[A, B]` 时，`r = 0` 得到 `[B, A]`，`r = 2^32 - 1` 保持 `[A, B]`。

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
    givm::card_table table{};
    const auto a = ids.get_id<givm::card_definition>("first");
    const auto b = ids.get_id<givm::card_definition>("second");
    auto player = table[givm::player_id{ 0 }];
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .cards = { a, b } });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(library, table, random);
    std::println("洗牌后牌数: {}", player.deck_card_count());
    std::println("原底牌变为顶牌: {}", player.deck_card_definition(1).value() == a.value());
}
```

输出

```text
洗牌后牌数: 2
原底牌变为顶牌: true
```
