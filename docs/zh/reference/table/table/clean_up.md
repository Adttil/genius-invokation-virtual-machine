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
#include <utility>
#include <array>
#include <cstddef>
#include <bitset>
#include <cstdint>
#include <print>
#include <ranges>
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
