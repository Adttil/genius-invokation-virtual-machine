[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **replace_cards**

# givm::replace_cards

定义于头文件 `<givm/definition.hpp>`

```cpp
struct replace_cards;
```

单方换牌命令，包括选择待换手牌、放回所选牌及抽取等量新牌。抽取时优先避开本次换回的同名牌。

## 成员类型

| | |
| --- | --- |
| [`error_type`](#编译检查) | `replace_cards_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 替换手牌的玩家 |

## 编译检查

```cpp
struct replace_cards_error;
```

`replace_cards::error_type` 是 `givm::replace_cards_error` 的别名。`replace_cards_error` 是本命令的结构化编译错误，`replace_cards_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `invalid_player` | `player.index()` 不是固定席位 `0` 或 `1` |

### `replace_cards_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错的 `player.index()` |

仅与当前 `cause` 对应的附加成员具有诊断含义。

## 注意

等待输入时，执行器返回 `execution_state::card_selection`，通过相应的[现场视图](../../executor/execution_view/card_selection.md)提交选择。选择位按换牌前有效手牌的遍历顺序，空选择不改变手牌与牌堆。

### 替换结果

设本次选择了 `k` 张牌：

1. 按手牌遍历顺序，逐张把选中的牌放回牌堆。每张取得一个 `std::uint32_t` 随机值 `r`；若放回前牌堆有 `n` 张牌，插入位置为 `floor(r × (n + 1) / 2^32)`，从牌堆底部的 `0` 数到顶部的 `n`。乘除按数学整数计算，`r` 的范围为 `0` 至 `2^32 - 1`；空牌堆也使用一个随机值。
2. 全部放回后，从牌堆顶至底优先选取定义 ID 不属于本次所选牌的 `k` 张牌；若不足 `k` 张，再从顶至底选取所选定义的牌补足。
3. 将选中的这 `k` 张牌按它们在牌堆中从顶至底的顺序取出并补入手牌；未抽到的牌保持相对次序。抽取不再使用随机值。

避免换回同一定义优先于牌的位置；需要补足时，同一定义的牌仍可能被抽回。

### 随机调用与事件

首次返回 `card_selection` 前不调用随机源。提交选择并继续推进后，按上述放回顺序取得恰好 `k` 个值。替换全部完成后，再按补入手牌的顺序发出 [`card_drawn`](../events/card_drawn.md)。事件响应及其后续效果可以继续调用同一随机源，这些调用不包含在上述 `k` 次之内。

## 示例

```cpp
#include <utility>
#include <array>
#include <cstddef>
#include <bitset>
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct card_source
{
    static constexpr auto category = givm::definition_category::card;
    struct definition_type {};
    std::string_view source_name;
    std::string_view name() const { return source_name; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    card_source first{ "first" };
    card_source second{ "second" };
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(first, second)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{ givm::draw_cards{ .position = 0, .count = 1 }, givm::draw_cards{ .player = givm::relative_player::opponent, .position = 0, .count = 1 }, givm::replace_cards{ .player = givm::player_id{ 0 } } },
        std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 0, .self_player = givm::player_id{ 0 } } };
    const auto a = ids.get_id<givm::definition_category::card>("first");
    const auto b = ids.get_id<givm::definition_category::card>("second");
    load_deck(table, library,
        givm::linked_deck{ .cards = { b, a } },
        givm::linked_deck{ .cards = { b, a } });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    std::bitset<givm::selection_capacity> selected{};
    selected.set(0);
    execution.view_in<givm::execution_state::card_selection>().select(library, table, random, selected);
    std::println("玩家 0 的手牌数量: {}", table[givm::player_id{ 0 }].hand_card_count());
    std::println("玩家 0 抽到另一种牌: {}",
        (*table[givm::player_id{ 0 }].hand_cards().begin()).definition_id().value() == b.value());
}
```

输出

```text
玩家 0 的手牌数量: 1
玩家 0 抽到另一种牌: true
```

## 参阅

| | |
| --- | --- |
| [`card_drawn`](../events/card_drawn.md) | 一张牌抽取完成后的通知 |
