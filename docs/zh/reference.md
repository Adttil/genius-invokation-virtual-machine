# Genius Invokation Virtual Machine 参考

## [枚举值](reference/enums.md)

| | |
| --- | --- |
| [`element`](reference/enums/element.md) | 元素 |
| [`elemental_reaction`](reference/enums/elemental_reaction.md) | 元素反应 |
| [`weapon_type`](reference/enums/weapon_type.md) | 武器类型 |
| [`game_result`](reference/enums/game_result.md) | 对局结果 |

## [定义](reference/definition.md)

| | |
| --- | --- |
| [`definition_source_view`](reference/definition/definition_source_view.md) | 定义源的类型擦除视图 |
| [`definition_source_library`](reference/definition/definition_source_library.md) | 定义源库 |
| [`definition_library`](reference/definition/definition_library.md) | 定义库 |

## [牌桌](reference/table.md)

| | |
| --- | --- |
| [`table`](reference/table/table.md) | 牌桌 |
| [`definition_id`](reference/table/definition_id.md) | 实体采用的定义身份 |
| [`linked_deck`](reference/table/linked_deck.md) | 已确定卡牌与角色定义的牌组 |
| [`player_view`](reference/table/player_view.md) | 玩家的只读视图 |
| [`hand_card_view`](reference/table/hand_card_view.md) | 手牌的只读视图 |
| [`deck_card_view`](reference/table/deck_card_view.md) | 牌库卡牌的只读视图 |
| [`character_view`](reference/table/character_view.md) | 角色的只读视图 |

## [执行](reference/executor.md)

| | |
| --- | --- |
| [`executor`](reference/executor/executor.md) | 游戏对局的执行器 |
| [`execution_state`](reference/executor/execution_state.md) | 对局执行现场的种类 |
| [`execution_view`](reference/executor/execution_view.md) | 对局执行现场的视图 |
| [`random_fn`](reference/executor/random_fn.md) | 随机函数视图 |
| [事件](reference/executor/events.md) | 可响应的游戏事件 |
| [指令](reference/executor/instructions.md) | 游戏规则的操作描述 |

## [通用工具](reference/utils.md)

| | |
| --- | --- |
| [`type_list`](reference/utils/type_list.md) | 类型列表 |
| [`frame_stack`](reference/utils/frame_stack.md) | 帧栈 |
