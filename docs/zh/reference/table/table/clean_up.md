[givm](../../../reference.md) / [牌桌](../../table.md) / [table](../table.md) / **clean_up**

# givm::table::clean_up

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr void clean_up() noexcept;
```

清理双方已经移除的实体及卡牌状态，保留尚未移除的实体。

清理会回收已删除实体保留的信息，结束通过原有 ID 读取这些信息的期限。

## 返回值

（无）

## 注意

清理可能改变实体 ID，并使已取得的实体访问对象、范围和状态引用失效。清理后应重新从牌桌获取它们；不要在结算仍持有这些对象时调用。牌库中尚未移除卡牌的先后顺序保持不变。


## 示例

```cpp
#include <bitset>
#include <cstdint>
#include <print>
#include <ranges>
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
        std::tuple{ givm::draw_cards{ .count = 1 }, givm::draw_cards{ .count = 1, .player = givm::relative_player::other }, givm::replace_cards{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
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
    execution.view_in<givm::execution_state::card_selection>().select(selected);
    execution.step(library, table, random);
    std::println("清理前的手牌槽位数: {}", std::ranges::distance(table[givm::player_id{ 0 }].hand_cards<false>()));
    table.clean_up();
    std::println("清理后的手牌槽位数: {}", std::ranges::distance(table[givm::player_id{ 0 }].hand_cards<false>()));
    std::println("保留的手牌张数: {}", table[givm::player_id{ 0 }].hand_card_count());
}
```

输出

```text
清理前的手牌槽位数: 2
清理后的手牌槽位数: 1
保留的手牌张数: 1
```
