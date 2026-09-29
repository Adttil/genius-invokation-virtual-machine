[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **insert_deck_card**

# givm::insert_deck_card

定义于头文件 `<givm/definition.hpp>`

```cpp
struct insert_deck_card;
```

向牌堆插入指定牌的命令。它适用于准备初始牌堆，或由游戏效果向牌堆添加指定牌。

## 成员类型

| | |
| --- | --- |
| [`error_type`](#编译检查) | `insert_deck_card_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 接收牌的玩家 |
| `definition` | `definition_id<card_definition>` | 要插入的牌定义 |
| `position` | `std::int32_t` | 插入位置，初始为 -1，即牌堆顶 |

## 编译检查

```cpp
struct insert_deck_card_error;
```

`insert_deck_card::error_type` 是 `givm::insert_deck_card_error` 的别名。`insert_deck_card_error` 是本命令的结构化编译错误，`insert_deck_card_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `invalid_player` | `player.index` 不是固定席位 `0` 或 `1` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `card_definition` 定义数量 |

### `insert_deck_card_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错的 `player.index` 或定义 ID 的 `value()` |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。

## 注意

新牌采用其定义已保存的 [`card_initial_state`](../queries/card_initial_state.md) 查询结果，包含卡牌自身费用与元素调和许可。

位置非负时，从牌堆底起计数，`0` 表示最底端，牌堆大小表示最顶端；负数从顶端计数，`-1` 表示顶端、`-2` 表示顶端下一张的位置。位置必须落在现有牌之间或两端。本命令不调用随机源。

## 示例

```cpp
#include <array>
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>
#include <utility>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;
    struct definition_type {};
    std::string_view name() const { return "first"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

struct effect_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "second"; }
    auto card_dependencies() const
    { return std::array<std::string_view, 1>{ "first" }; }

    givm::program_entry compile(givm::definition_compile_context& context) const
    {
        const auto definition = context.resolve_id<givm::card_definition>("first");
        return context.add_program(
            givm::insert_deck_card{ .player = givm::player_id{ 0 }, .definition = definition });
    }

    static givm::program_entry handle(const givm::program_entry& entry, const givm::deck_card_view&,
        givm::round_started&, givm::handle_context& context)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const card_source first{};
    const effect_source second{};
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0,
        givm::genshin_impact::shield_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(first, second)) return 1;
    auto library_result = compile(sources, basics, std::tuple{},
        std::tuple{
            givm::start_round{},
            givm::end_game{ .result = givm::game_result::both_loss }
        }, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{};
    load_deck(table, library,
        givm::linked_deck{ .cards = { ids.get_id<givm::card_definition>("second") } },
        givm::linked_deck{});
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    const auto player = table[givm::player_id{ 0 }];
    const auto card = ids.get_id<givm::card_definition>("first");
    std::println("牌堆数量: {}", player.deck_card_count());
    std::println("插入指定牌: {}", player.deck_card_definition(1) == card);
}
```

输出

```text
牌堆数量: 2
插入指定牌: true
```
