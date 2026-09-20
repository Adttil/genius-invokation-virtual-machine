[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **insert_deck_card**

# givm::insert_deck_card

定义于头文件 `<givm/definition.hpp>`

```cpp
struct insert_deck_card;
```

向牌堆插入指定牌的命令。它适用于准备初始牌堆，或由游戏效果向牌堆添加指定牌。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 接收牌的玩家 |
| `definition` | `definition_id<card_definition>` | 要插入的牌定义 |
| `position` | `std::int32_t` | 插入位置，初始为 -1，即牌堆顶 |

## 注意

位置非负时，从牌堆底起计数，`0` 表示最底端，牌堆大小表示最顶端；负数从顶端计数，`-1` 表示顶端、`-2` 表示顶端下一张的位置。位置必须落在现有牌之间或两端。本命令不调用随机源。

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
    const auto issued = sources.make_issued_id_map();
    const auto card = issued.get_id<givm::card_definition>("first");
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::insert_deck_card{ .player = givm::player_id{ 0 }, .definition = card } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    std::println("牌堆数量: {}", table[givm::player_id{ 0 }].deck_card_count());
    std::println("插入指定牌: {}", table[givm::player_id{ 0 }].deck_card_definition(0).value() == card.value());
}
```

输出

```text
牌堆数量: 1
插入指定牌: true
```
