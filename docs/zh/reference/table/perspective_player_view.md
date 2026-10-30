# givm::perspective_player_view

定义于头文件 `<givm/table.hpp>`

```cpp
template<unsigned Player, role Role>
class perspective_player_view;
```

按静态归属读取一位玩家的公开实体和允许查看的私有信息。由 [`perspective_table_view::player`](perspective_table_view/player.md) 创建。

| 成员 | 用途 |
| --- | --- |
| [`id`](perspective_player_view/id.md) | 返回静态关联玩家的ID。 |
| [`active_character_id`](perspective_player_view/active_character_id.md) | 返回当前出战角色的可空ID。 |
| [`dice_count`](perspective_player_view/dice_count.md) | 返回骰子总数，双方玩家及观众均可读取。 |
| [`dice`](perspective_player_view/dice.md) | 仅本人、观众或全权身份提供，返回各色骰子数量的只读引用。 |
| [`hand_limit`](perspective_player_view/hand_limit.md) | 读取玩家的手牌上限。 |
| [`summon_limit`](perspective_player_view/summon_limit.md) | 读取玩家的召唤物上限。 |
| [`support_limit`](perspective_player_view/support_limit.md) | 读取玩家的支援上限。 |
| [`can_plunge`](perspective_player_view/can_plunge.md) | 读取当前是否仍有下落攻击资格。 |
| [`hand_cards`](perspective_player_view/hand_cards.md) | 本人及观众取得普通 `hand_card_view`，对手取得 `hidden_hand_card_view`。默认跳过已移除的手牌，不复制集合。 |
| [`operator[]`](perspective_player_view/operator_at.md) | 按ID读取手牌，返回类型遵循 `hand_cards()` 的规则。ID必须属于本玩家；Debug断言该前提，Release不检查。 |

`hand_card_count()`、`deck_card_count()` 读取数量；`characters()`、`supports()`、`summons()`、`combat_statuses()`、`reaction(slot)` 转发现有公开实体读取。集合接口保留 `SkipErased` 模板参数。

本视图通过字段getter读取玩家状态；各角色都不提供包含完整骰子颜色的原始 `player_state` 或逐牌牌堆接口。`category` 与 `access_role` 为静态常量。
