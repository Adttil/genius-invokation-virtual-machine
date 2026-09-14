[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **deal_damage**

# givm::deal_damage

定义于头文件 `<givm/executor/instructions/deal_damage.hpp>`

```cpp
struct deal_damage;
```

伤害结算指令。它为增伤、减伤和护盾等效果提供响应时机，并在扣除生命后处理伤害所携带的元素附着与胜负判定。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`damage_source_id`](../events/damage_source_id.md) | 造成伤害的来源 |
| `target` | [`character_id`](../../table/character_id.md) | 受到伤害的有效角色 |
| `value` | `std::uint32_t` | 初始伤害值 |
| `multiplier_numerator` | `std::uint16_t` | 初始伤害倍率的分子，默认为 1 |
| `multiplier_denominator` | `std::uint16_t` | 初始伤害倍率的非零分母，默认为 1 |
| `type` | [`damage_type`](../../enums/damage_type.md) | 伤害种类 |
| `flags` | [`damage_flags`](../../enums/damage_flags.md) | 伤害附加属性 |

## 注意

依次经过 [`damage_calculation`](../events/damage_calculation.md)、[`damage_effect`](../events/damage_effect.md) 和 [`after_damage`](../events/after_damage.md)。计算倍率时向下取整，超出 uint32_t 范围时取其最大值；扣除生命不会低于 0。伤害后的响应结束后，根据双方是否仍有存活角色判断胜负。

以 [`step`](../executor/step.md) 推进时，非零最终伤害扣除生命后先返回 `execution_state::health_reduced`；随后推进才处理元素附着及伤害后响应。相应[视图](../execution_view/health_reduced.md)的伤害值不以目标原有生命为上限。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct character_source
{
    using definition_category = givm::character_view;
    struct definition_type {};
    std::string_view name() const { return "character"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::program_entry<givm::character_initialization> handle(
        const definition_type&, const givm::character_view&,
        givm::character_initialization& event, const givm::card_table&, givm::random_fn&)
    {
        event.state = { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
        return givm::program_entry<givm::character_initialization>::null();
    }
};

int main()
{
    character_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::initialize_characters{ .player = givm::player_id{ 0 } }, givm::initialize_characters{ .player = givm::player_id{ 1 } }, givm::deal_damage{ .source = givm::character_id{ .player_id = givm::player_id{ 0 }, .index = 0 }, .target = { .player_id = givm::player_id{ 1 }, .index = 0 }, .value = 3, .type = givm::damage_type::physical, .flags = {} } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{};
    const auto definition = ids.get_id<givm::character_view>("character");
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .characters = { definition } });
    table.load_deck(givm::player_id{ 1 }, givm::linked_deck{ .characters = { definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(library, table, random);
    std::println("目标剩余生命: {}", table[target].state().health);
}
```

输出

```text
目标剩余生命: 7
```

## 参阅

| | |
| --- | --- |
| [`damage_calculation`](../events/damage_calculation.md) | 伤害计算事件 |
| [`damage_effect`](../events/damage_effect.md) | 扣除生命前的伤害结算事件 |
| [`after_damage`](../events/after_damage.md) | 伤害及其元素附着结算完成后的通知 |
