[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **replace_cards_both**

# givm::replace_cards_both

定义于头文件 `<givm/definition.hpp>`

```cpp
struct replace_cards_both;
```

双方开局换牌命令。每方提交后即可得到该方的替换结果；双方都提交后结束这项准备。

## 注意

尚未接受任何一方的选择时，执行器返回 `execution_state::initial_card_selection`，通过相应的[现场视图](../../executor/execution_view/initial_card_selection.md)指定首先换牌的玩家及其选择。该方换牌完成后返回 `execution_state::card_selection`，由相应[视图](../../executor/execution_view/card_selection.md)读取剩余玩家并提交其选择。

可以任选先提交的一方；选择位按该方换牌前有效手牌的遍历顺序。空选择不改变该方手牌与牌堆。这个双方开局替换过程不发出 [`card_drawn`](../events/card_drawn.md)。

### 随机值的预先分配

设开始时两方有效手牌数为 `H0` 和 `H1`。首次返回 `initial_card_selection` 前，先为玩家 0 取得 `H0` 个 `std::uint32_t` 随机值，再为玩家 1 取得 `H1` 个值，总计 `H0 + H1` 次调用。

某方选择 `k` 张牌时，按手牌遍历顺序使用分配给该方的前 `k` 个值；不是按选中牌原来的手牌位置挑选对应随机值。该方剩余的值弃用。提交选择后，换牌本身不再调用随机源。首先提交哪一方，不改变各方分配到的数值或替换结果。

这些已取得的值随执行现场一同复制；后续推进改用其他随机源不会改变它们。记录回放时应计入全部 `H0 + H1` 次调用，包括最终弃用的值。

### 替换结果

每方提交选择后，设所选牌数为 `k`：

1. 按手牌遍历顺序，逐张把选中的牌放回该方牌堆。每张使用一个预先分配的随机值 `r`；若放回前牌堆有 `n` 张牌，插入位置为 `floor(r × (n + 1) / 2^32)`，从牌堆底部的 `0` 数到顶部的 `n`。乘除按数学整数计算，`r` 的范围为 `0` 至 `2^32 - 1`；空牌堆也使用一个已分配的值。
2. 全部放回后，从牌堆顶至底优先选取定义 ID 不属于本次所选牌的 `k` 张牌；若不足 `k` 张，再从顶至底选取所选定义的牌补足。
3. 将选中的这 `k` 张牌按它们在牌堆中从顶至底的顺序取出并补入手牌；未抽到的牌保持相对次序。抽取不再使用随机值。

避免换回同一定义优先于牌的位置；需要补足时，同一定义的牌仍可能被抽回。

## 示例

```cpp
#include <bitset>
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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(first, second);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::draw_cards{ .count = 1 }, givm::draw_cards{ .count = 1, .player = givm::relative_player::other }, givm::replace_cards_both{} },
        std::tuple{}, givm::compile_mode::normal);
    givm::table table{ { .max_rounds = 0 } };
    const auto a = ids.get_id<givm::card_definition>("first");
    const auto b = ids.get_id<givm::card_definition>("second");
    load_deck(table, library,
        givm::linked_deck{ .cards = { b, a } },
        givm::linked_deck{ .cards = { b, a } });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    std::bitset<givm::selection_capacity> selected{};
    selected.set(0);
    execution.view_in<givm::execution_state::initial_card_selection>().select(givm::player_id{ 1 }, selected);
    execution.step(library, table, random);
    const auto remaining = execution.view_in<givm::execution_state::card_selection>();
    std::println("剩余玩家为玩家 0: {}", remaining.player() == givm::player_id{ 0 });
    remaining.select(selected);
    execution.step(library, table, random);
    std::println("玩家 0 的手牌数量: {}", table[givm::player_id{ 0 }].hand_card_count());
    std::println("玩家 0 抽到另一种牌: {}",
        (*table[givm::player_id{ 0 }].hand_cards().begin()).definition_id().value() == b.value());
}
```

输出

```text
剩余玩家为玩家 0: true
玩家 0 的手牌数量: 1
玩家 0 抽到另一种牌: true
```
