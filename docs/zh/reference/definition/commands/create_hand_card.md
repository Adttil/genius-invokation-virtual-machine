[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **create_hand_card**

# givm::create_hand_card

定义于头文件 `<givm/definition.hpp>`

```cpp
struct create_hand_card
{
    relative_player player = relative_player::self;
    definition_id<card_definition> definition{};
};
```

直接向一位玩家的手牌中生成一张指定牌的命令。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`relative_player`](relative_player.md) | 固定模式下接收新牌的一方，默认为本方 |
| `definition` | `definition_id<card_definition>` | 固定模式下新牌的定义；默认采用动态输入 |

## 注意

默认构造 `create_hand_card{}` 使用动态模式，由响应通过 `invoke` 提交一个 [`hand_card_creation`](../events/hand_card_creation.md)。显式指定 `definition` 时采用固定模式，不消费响应输入。动态输入的玩家与定义必须有效。

命令执行时读取接收玩家当前的 [`player_state::hand_limit`](../../table/player_state.md)。手牌已达到或超过上限时，本次生成无效，不创建实体、不发送通知，也不属于舍弃。

存在空位时，新牌采用其定义的 [`card_initial_state`](../queries/card_initial_state.md)，加入手牌后全场广播 [`hand_card_added`](../events/hand_card_added.md)，其响应及效果结算完毕后再继续后续命令。

生成手牌不属于抽牌，不发出 [`card_drawn`](../events/card_drawn.md)。需要响应所有加入手牌情形的定义，应同时响应 `hand_card_added` 与 `card_drawn`。命令本身不调用随机源；通知响应产生的效果可以使用随机源。

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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto issued = sources.make_issued_id_map();
    const auto definition = issued.get_id<givm::card_definition>("生成示例牌");
    const auto [library, ids] = compile(sources,
        std::tuple{
            givm::create_hand_card{ .definition = definition },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, givm::compile_mode::normal);

    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
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
| [`hand_card_creation`](../events/hand_card_creation.md) | 生成手牌的动态输入 |
| [`hand_card_added`](../events/hand_card_added.md) | 非抽牌方式加入手牌后的通知 |
| [`draw_cards`](draw_cards.md) | 从牌堆抽牌的命令 |
