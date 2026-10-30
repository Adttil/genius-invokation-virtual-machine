# givm::hidden_hand_card_view

定义于头文件 `<givm/table.hpp>`

对手手牌的只读视图，身份隐藏但仍保留真实的 `hand_card_id`。由权限玩家视图的手牌集合或索引产生。

| 成员 | 用途 |
| --- | --- |
| [`id`](hidden_hand_card_view/id.md) | 读取真实手牌ID |
| [`is_valid`](hidden_hand_card_view/is_valid.md) | 读取是否仍有效 |
| [`operator bool`](hidden_hand_card_view/operator_bool.md) | 按有效性判定 |
| [`statuses`](hidden_hand_card_view/statuses.md) | 读取公开附着状态 |

不提供 `definition_id()` 和 `state()`。公开标签通过 [`definition_library::public_tags`](../executor/definition_library/public_tags.md) 读取。实体类别仍为 `entity_category::hand_card`。
