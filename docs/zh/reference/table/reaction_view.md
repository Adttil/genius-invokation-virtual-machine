[givm](../../reference.md) / [牌桌](../table.md) / **reaction_view**

# givm::reaction_view

定义于头文件 `<givm/table.hpp>`

某位玩家一个反应槽位的只读视图，通过 `table[reaction_id{player, slot}]` 取得。它没有可变实体状态，具体定义由装载牌组后的反应映射决定，不因角色死亡而消失。

| 成员 | 说明 |
| --- | --- |
| [`id`](reaction_view/id.md) | 返回玩家和槽位组成的 `reaction_id` |
| [`player`](reaction_view/player.md) | 返回映射所属玩家的视图 |
| [`slot`](reaction_view/slot.md) | 返回基础反应槽位 |
| [`definition_id`](reaction_view/definition_id.md) | 返回该方此槽位的反应定义 ID |
| [`table`](reaction_view/table.md) | 返回所属牌桌的 `const table&` |
| [`is_valid`](reaction_view/is_valid.md)、[`operator bool`](reaction_view/operator_bool.md) | 检查视图是否有效 |

反应发生在某方角色上时，使用其对方玩家的映射；引发来源玩家可从反应事件的 `source_player()` 取得。反应定义可以据此定制效果归属。
