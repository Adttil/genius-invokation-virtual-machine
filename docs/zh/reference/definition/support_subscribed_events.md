[givm](../../reference.md) / [定义](../definition.md) / **support_subscribed_events**

# givm::support_subscribed_events

定义于头文件 `<givm/definition.hpp>`

```cpp
using support_subscribed_events = type_list<
    test_event,
    action_phase_started,
    battle_started,
    before_action,
    round_end_declared,
    round_ended,
    dice_roll_preparation,
    dice_added,
    dice_removed,
    dice_converted,
    changing_secret_points,
    secret_points_changed,
    changing_energy,
    energy_changed,
    calculating_card_payment,
    calculating_skill_payment,
    calculating_switch_payment,
    cost_of_card,
    cost_of_skill,
    cost_of_switch,
    hand_card_created,
    card_drawn,
    card_discarded,
    card_candidate_chosen,
    elemental_tuning_modification,
    elemental_tuning_completed,
    card_will_be_played,
    card_played,
    active_character_changed,
    skill_will_be_used,
    skill_used,
    damage_calculation,
    damage_effect,
    after_damage,
    healing,
    healed,
    elemental_reaction_will_occur,
    after_elemental_reaction,
    character_will_be_defeated,
    character_defeated,
    summon_removed,
    combat_status_removed,
    attachment_removed
>;
```

支援及多种场上实体共用的可订阅事件列表。它用于按实体类型组织事件处理能力，并不规定实际广播时的实体集合或顺序。

## 注意

列表操作见 [`type_list`](../utils/type_list.md)。角色初始状态由独立的 [`character_initial_state`](queries/character_initial_state.md) 查询提供。

## 示例

```cpp
#include <concepts>
#include <cstddef>
#include <print>

#include <givm/givm.hpp>

int main()
{
    std::size_t damage_related = 0;
    givm::support_subscribed_events::each([&]<class TEvent>
    {
        if constexpr(std::same_as<TEvent, givm::damage_calculation>
            || std::same_as<TEvent, givm::damage_effect>
            || std::same_as<TEvent, givm::after_damage>)
        {
            ++damage_related;
        }
    });
    std::println("可订阅的伤害结算事件数量: {}", damage_related);
}
```

输出

```text
可订阅的伤害结算事件数量: 3
```
