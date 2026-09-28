[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **create_hand_card**

# givm::create_hand_card

定义于头文件 `<givm/definition.hpp>`

```cpp
struct create_hand_card_error;

struct create_hand_card
{
    using error_type = create_hand_card_error;

    using input_type = create_hand_card_input;

    relative_player player = relative_player::self;
    definition_id<card_definition> definition{};
};
```

直接向一位玩家的手牌中生成一张指定牌的命令。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`create_hand_card_input`](../command_inputs/create_hand_card_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `create_hand_card_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`relative_player`](relative_player.md) | 固定模式下接收新牌的一方，默认为本方 |
| `definition` | `definition_id<card_definition>` | 固定模式下新牌的定义；默认采用动态输入 |

## 编译检查

```cpp
struct create_hand_card_error;
```

`create_hand_card::error_type` 是 `givm::create_hand_card_error` 的别名。`create_hand_card_error` 是本命令的结构化编译错误，`create_hand_card_error::reason` 是原因枚举。[编译检查 `check`](../../executor/check.md) 使用本次定义集合与程序种类检查以下条件；[`compile`](../../executor/compile.md) 自动收集这些错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `card_definition` 定义数量 |

### `create_hand_card_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 注意

默认构造 `create_hand_card{}` 使用动态模式，由响应通过 `invoke` 提交一个 [`create_hand_card_input`](../command_inputs/create_hand_card_input.md)。显式指定 `definition` 时采用固定模式，不消费响应输入。动态输入的玩家与定义必须有效。

命令执行时读取接收玩家当前的 [`player_state::hand_limit`](../../table/player_state.md)。手牌已达到或超过上限时，本次生成无效，不创建实体、不发送通知，也不属于舍弃。

存在空位时，新牌采用其定义的 [`card_initial_state`](../queries/card_initial_state.md)，加入手牌后全场广播 [`hand_card_added`](../events/hand_card_added.md)，其响应及效果结算完毕后再继续后续命令。

生成手牌不属于抽牌，不发出 [`card_drawn`](../events/card_drawn.md)。需要响应所有加入手牌情形的定义，应同时响应 `hand_card_added` 与 `card_drawn`。命令本身不调用随机源；通知响应产生的效果可以使用随机源。

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
    std::string_view name() const { return "生成示例牌"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::card_state query(const definition_type&, const givm::card_initial_state&)
    {
        return { .cost = { .energy = 2 }, .elemental_tuning_allowed = false };
    }
};

int main()
{
    const card_source source{};
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(source)) return 1;
    auto issued_result = sources.make_issued_id_map(basics);
    if(not issued_result)
    {
        std::println("{}", error_string(issued_result.error()));
        return 1;
    }
    const auto issued = std::move(*issued_result);
    const auto definition = issued.get_id<givm::card_definition>("生成示例牌");
    auto library_result = compile(sources, basics,
        std::tuple{
            givm::create_hand_card{ .definition = definition },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);

    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.start(library, table);
    execution.step(library, table, random);
    const auto player = table[givm::player_id{ 0 }];
    const auto card = *player.hand_cards().begin();
    std::println("手牌数量: {}", player.hand_card_count());
    std::println("生成的牌: {}", library[card.definition_id()].name());
    std::println("自身充能费用: {}", card.state().cost.energy);
    std::println("允许元素调和: {}", card.state().elemental_tuning_allowed);
}
```

输出

```text
手牌数量: 1
生成的牌: 生成示例牌
自身充能费用: 2
允许元素调和: false
```

## 参阅

| | |
| --- | --- |
| [`create_hand_card_input`](../command_inputs/create_hand_card_input.md) | 生成手牌的动态输入 |
| [`hand_card_added`](../events/hand_card_added.md) | 非抽牌方式加入手牌后的通知 |
| [`draw_cards`](draw_cards.md) | 从牌堆抽牌的命令 |
