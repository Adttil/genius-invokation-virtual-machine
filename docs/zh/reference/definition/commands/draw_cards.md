[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **draw_cards**

# givm::draw_cards

定义于头文件 `<givm/definition.hpp>`

```cpp
struct draw_cards_error;

struct draw_cards
{
    using error_type = draw_cards_error;

    using input_type = draw_cards_input;

    relative_player player = relative_player::self;
    std::span<const std::size_t> positions{};
};
```

从牌堆抽取一组牌的命令。可以按相对牌堆顶的位置抽牌，也可以由响应指定具体的牌堆卡牌，保留它们原有的状态和附属状态。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`draw_cards_input`](../command_inputs/draw_cards_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `draw_cards_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`relative_player`](relative_player.md) | 固定模式下相对于当前效果本方的抽牌方，初始为 `self` |
| `positions` | `std::span<const std::size_t>` | 固定模式下按抽取顺序排列的牌堆顶相对位置，初始为空 |

## 编译检查

```cpp
struct draw_cards_error;
```

`draw_cards::error_type` 是 `givm::draw_cards_error` 的别名。`draw_cards_error` 是本命令的结构化编译错误，`draw_cards_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `duplicate_position` | `positions[index]` 与之前的位置重复；本次抽取的固定位置不得重复 |

### `draw_cards_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | `invalid_player` 时为玩家枚举的数值；`duplicate_position` 时为重复的牌堆位置 |
| `index` | `std::size_t` | `duplicate_position` 中重复元素在 `positions` 内从零开始的索引 |
| `first_index` | `std::size_t` | `duplicate_position` 中相同位置首次出现在 `positions` 内的索引 |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 注意

`positions` 非空时采用固定模式。所有位置都相对于本命令开始执行时的牌堆顶，`0` 表示顶牌；取走前面的目标不会改变后续位置的含义。例如 `{ 3, 1 }` 依次抽取原来的第四张、第二张牌。超出牌堆范围的位置跳过，位置不得重复。位置数组须在编译命令期间保持有效，编译完成后不再借用原数组。

默认构造 `draw_cards{}` 时，消费响应通过 [`invoke`](../../executor/handle_context/invoke.md) 提交的一个 [`draw_cards_input`](../command_inputs/draw_cards_input.md)。其中 `cards` 按抽取顺序指定有效的牌堆卡牌，允许为空，目标不得重复。每张牌进入其所属玩家的手牌，允许同一批指定双方的牌；动态模式不读取命令的 `player`。

固定模式下，响应程序中的本方是响应实体所属玩家。根流程使用固定模式时，须显式设置 [`table_state::self_player`](../../table/table_state.md)；下例设为玩家 0。

按指定顺序依次加入手牌，保留未抽取牌的相对顺序。达到所属玩家的手牌上限后，仍继续从牌堆移走本次应抽的牌，但这些牌不进入手牌，其附属状态也会移除。这种移除不属于舍弃，不触发舍弃效果或舍弃通知。

先完成本次所有抽牌，再按输入顺序逐张发出 [`card_drawn`](../events/card_drawn.md)，只通知实际进入手牌的牌。每张牌的通知及其响应程序全部结束后，才开始下一张牌的通知。

抽牌不额外广播 [`hand_card_added`](../events/hand_card_added.md)。响应任意方式加入手牌的定义，通过响应 `card_drawn` 参与同一次抽牌通知，与仅响应抽牌的定义按广播顺序共同结算。

## 示例

```cpp
#include <utility>
#include <array>
#include <cstddef>
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
        givm::genshin_impact::frozen_3_3_0,
        givm::genshin_impact::shield_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(first, second)) return 1;
    constexpr std::array<std::size_t, 2> draw_positions{ 0, 1 };
    auto library_result = compile(
        sources, basics,
        std::tuple{ givm::draw_cards{ .positions = draw_positions } },
        std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 0, .self_player = givm::player_id{ 0 } } };
    auto player = table[givm::player_id{ 0 }];
    const auto card = ids.get_id<givm::card_definition>("first");
    load_deck(table, library, givm::linked_deck{ .cards = { card, card } }, {});
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    std::println("手牌数量: {}", player.hand_card_count());
    std::println("牌堆数量: {}", player.deck_card_count());
}
```

输出

```text
手牌数量: 2
牌堆数量: 0
```

## 参阅

| | |
| --- | --- |
| [`draw_cards_input`](../command_inputs/draw_cards_input.md) | 指定一组牌堆卡牌的动态输入 |
| [`card_drawn`](../events/card_drawn.md) | 一张牌抽取完成后的通知 |
