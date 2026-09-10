[givm](../../reference.md) / [定义](../definition.md) / **subscribed_events**

# givm::subscribed_events

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class EntityView>
struct subscribed_events;
```

一种实体形态可以响应的事件集合。它确定定义源可为这一 view 提供哪些事件处理；具体某项定义是否提供处理，还需通过定义的 `can_handle` 查询。

## 模板参数

|  |  |
| --- | --- |
| `EntityView` | 受支持的只读实体 view |

## 注意

这些特化提供 [`type_list`](../utils/type_list.md) 的操作。未列出的 view 没有定义通用订阅集合；订阅能力也不规定某次事件必须广播给所有可订阅实体。

## 支持的实体形态

|  |  |
| --- | --- |
| [`hand_card_view`](../table/hand_card_view.md)、[`hand_card_status_view`](../table/hand_card_status_view.md) | [`test_event`](../events/test_event.md)、[`action_phase_started`](../events/action_phase_started.md)、[`battle_started`](../events/battle_started.md)、[`before_action`](../events/before_action.md)、[`round_end_declared`](../events/round_end_declared.md)、[`round_ended`](../events/round_ended.md)、[`dice_roll_preparation`](../events/dice_roll_preparation.md)、[`calculating_card_payment`](../events/calculating_card_payment.md)、[`hand_card_created`](../events/hand_card_created.md)、[`card_drawn`](../events/card_drawn.md)、[`card_discarded`](../events/card_discarded.md)、[`elemental_tuning_will_apply`](../events/elemental_tuning_will_apply.md)、[`elemental_tuning_applied`](../events/elemental_tuning_applied.md)、[`card_will_be_played`](../events/card_will_be_played.md)、[`card_played`](../events/card_played.md)、[`active_character_changed`](../events/active_character_changed.md)、[`entity_will_leave`](../events/entity_will_leave.md)、[`entity_left`](../events/entity_left.md) |
| [`deck_card_view`](../table/deck_card_view.md)、[`deck_card_status_view`](../table/deck_card_status_view.md) | [`test_event`](../events/test_event.md)、[`action_phase_started`](../events/action_phase_started.md)、[`battle_started`](../events/battle_started.md)、[`before_action`](../events/before_action.md)、[`round_end_declared`](../events/round_end_declared.md)、[`round_ended`](../events/round_ended.md)、[`dice_roll_preparation`](../events/dice_roll_preparation.md)、[`card_drawn`](../events/card_drawn.md)、[`card_discarded`](../events/card_discarded.md)、[`card_played`](../events/card_played.md)、[`active_character_changed`](../events/active_character_changed.md)、[`entity_will_leave`](../events/entity_will_leave.md)、[`entity_left`](../events/entity_left.md) |
| [`support_view`](../table/support_view.md)、[`summon_view`](../table/summon_view.md)、[`combat_status_view`](../table/combat_status_view.md)、[`skill_view`](../table/skill_view.md)、[`attachment_view`](../table/attachment_view.md) | [`support_subscribed_events`](support_subscribed_events.md) 中的事件 |
| [`character_view`](../table/character_view.md) | [`support_subscribed_events`](support_subscribed_events.md) 中的事件，以及 [`character_initialization`](../events/character_initialization.md) |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    constexpr auto accepts_damage = []<class TView>()
    {
        return requires { givm::subscribed_events<TView>::template index_of<givm::damage_effect>(); };
    };
    std::println("支援可订阅伤害生效: {}", accepts_damage.template operator()<givm::support_view>());
    std::println("手牌可订阅伤害生效: {}", accepts_damage.template operator()<givm::hand_card_view>());
}
```

输出

```text
支援可订阅伤害生效: true
手牌可订阅伤害生效: false
```
