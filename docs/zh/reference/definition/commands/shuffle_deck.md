[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **shuffle_deck**

# givm::shuffle_deck

定义于头文件 `<givm/definition.hpp>`

```cpp
struct shuffle_deck;
```

洗牌命令，用于随机重排指定玩家的牌堆。牌的内容和牌堆数量保持不变。

## 成员类型

| | |
| --- | --- |
| [`error_type`](#编译检查) | `shuffle_deck_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 要洗牌的玩家 |

## 编译检查

```cpp
struct shuffle_deck_error;
```

`shuffle_deck::error_type` 是 `givm::shuffle_deck_error` 的别名。`shuffle_deck_error` 是本命令的结构化编译错误，`shuffle_deck_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `invalid_player` | `player.index` 不是固定席位 `0` 或 `1` |

### `shuffle_deck_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错的 `player.index` |

仅与当前 `cause` 对应的附加成员具有诊断含义。

## 注意

对 `N` 张牌调用随机源 `max(N - 1, 0)` 次。按牌序从底至顶编号为 `0` 至 `N - 1`，洗牌结果由以下规则确定：

1. 依次令 `m` 为 `N`、`N - 1`，直到 `2`。
2. 每次取得一个 `std::uint32_t` 随机值 `r`，令 `j = floor(r × m / 2^32)`。
3. 交换当前位置 `m - 1` 与位置 `j` 的牌。

这里的乘除按数学整数计算，`r` 的范围为 `0` 至 `2^32 - 1`。每次 `j` 都在 `0` 至 `m - 1` 内；即使两位置相同，也消耗该随机值。空牌堆和单张牌堆不调用随机源。例如两张牌由底至顶为 `[A, B]` 时，`r = 0` 得到 `[B, A]`，`r = 2^32 - 1` 保持 `[A, B]`。

## 示例

```cpp
#include <utility>
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
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(first, second)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 0 } };
    const auto a = ids.get_id<givm::card_definition>("first");
    const auto b = ids.get_id<givm::card_definition>("second");
    auto player = table[givm::player_id{ 0 }];
    load_deck(table, library, givm::linked_deck{ .cards = { a, b } }, {});
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    std::println("洗牌后牌数: {}", player.deck_card_count());
    std::println("原底牌变为顶牌: {}", player.deck_card_definition(1).value() == a.value());
}
```

输出

```text
洗牌后牌数: 2
原底牌变为顶牌: true
```
