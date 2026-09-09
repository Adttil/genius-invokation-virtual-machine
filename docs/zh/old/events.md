# 当前指令与事件目录

本页是现行 instruction/event 的权威目录，只列出当前由 `include/givm/executor/instructions.hpp` 导出的规则机制及其直接使用的事件。仓库中未列在本页的同名头文件和文档属于设计草案，不代表名称、字段、流程或边界已经确定。

每个链接页面规定相应类型的公开契约，包括字段与默认值、领域语义、执行顺序、事件顺序、挂起点以及可观察 stack ABI。实现和性能优化必须保持这些契约；页面不记录私有状态机写法或擦除存储布局。

共享的重入、activation、默认广播和 onpay 协议见 [指令、栈与事件系统](event-system.md)，stack 容器接口见 [Executor Stack](stack.md)，最小对局组合见 [最小可运行对局](game-flow.md)。执行层统一使用 instruction 术语；`test_command` 保留现有类型名。

## 公共结算约定

产生事件的领域指令负责建立事件页面规定的 frame、推进响应、读取最终 event、完成后处理并恢复入口栈形状。handler 可以修改 event，并返回空入口、固定响应程序或终局结果；终局保留现场供上层观察。具体广播 frame 只在共享协议页定义一次，各类型页面只记录相对于该协议的字段和特殊行为。

## 初始化工具

核心不固定初始化流程。上层按所需模式组合这些指令。

| 指令 | 机制 |
| --- | --- |
| [`insert_deck_card`](instructions/insert_deck_card.md) | 创建牌并按有符号位置插入牌堆。 |
| [`enter_character`](instructions/enter_character.md) | 创建并使一个角色入场。 |
| [`shuffle_deck`](instructions/shuffle_deck.md) | 随机重排指定玩家的牌堆逻辑顺序。 |
| [`initialize_characters`](instructions/initialize_characters.md) | 初始化指定玩家已经装入 table 的所有角色。 |
| [`set_active_character`](instructions/set_active_character.md) | 直接设置初始或规则强制的出战角色。 |
| [`select_active_character_both`](instructions/select_active_character_both.md) | 准备并推进开局双方出战角色选择；双方都提交后同时设置出战角色。 |

| Event | 机制 |
| --- | --- |
| [`character_initialization`](events/character_initialization.md) | 角色入场时用于生成初始状态的工作区。 |
| [`active_character_changed`](events/active_character_changed.md) | 出战角色已经被设置。 |

## 牌堆、抽牌与替换

[`draw_cards`](instructions/draw_cards.md) 和 [`replace_cards`](instructions/replace_cards.md) 是批量状态操作：先完成整个批次，再逐张广播 `card_drawn`。牌堆顶是抽牌首先读取的一端。开局双方替换通常直接使用 [`replace_cards_both`](instructions/replace_cards_both.md)：它预发插回随机数并压入一个 selector 输入槽，每次消费一名玩家选择并立即结算该方换牌，但同步流程内不广播 `card_drawn`。

`deck_card_id` 指向牌堆的稳定槽位，插牌或重排不会改变 cleanup 前仍存活牌实体的 ID。牌被抽取或删除后，原 ID 指向的槽位立即变为 invalid；`table.clean_up()` 会搬移并收缩槽位存储，此后此前保存的牌堆 ID 均不得继续使用。指令不携带它作为延迟目标；需要延迟处理牌堆牌时，应按牌定义或标签重新搜索。

| 指令 | 机制 |
| --- | --- |
| [`draw_cards`](instructions/draw_cards.md) | 整体抽取至多 N 张牌，再按顺序逐张通知。 |
| [`replace_cards`](instructions/replace_cards.md) | 准备单个玩家换牌输入槽，消费选择后随机插回并按同名回避规则抽取等量牌。 |
| [`replace_cards_both`](instructions/replace_cards_both.md) | 准备并推进双方开局替换手牌；消费栈顶单份 selector，使用栈上预发随机数组立即处理该玩家替换。 |

| Event | 机制 |
| --- | --- |
| [`card_drawn`](events/card_drawn.md) | 单张牌已经因抽牌进入手牌。 |

## 栈选择输入

多选输入通常表现为 executor stack 顶部的 `selector` 对象，包含玩家和 bitset 选择结果。选择既可以来自玩家输入，也可以由规则效果设置；消费选择的具体指令自己解释 bitset。需要外部输入的流程通过返回 `false` 主动挂起，并把 frame 尾部的完整对象暴露为输入槽。开局选择出战角色是单选，直接使用尾部 `character_id` 输入槽，不使用 `selector`。

## 回合推进

回合开始先通过可修改事件收集双方固定骰子和重投次数，再完成双方首次随机投骰并预发重投随机数组。重投阶段的栈顶保持一个 selector 作为输入槽；任一方提交后可先结算该方结果，双方都结束后进入行动阶段。`begin_action` 首次进入时广播行动阶段开始，并在同一条指令内反复提供行动机会。双方都宣告结束后进入回合程序中的 `end_round`；它切换下一回合先手、清空 `first_ended` 并广播回合结束。后续显式 `draw_cards` 槽完成抽牌，回合程序结束后重新进入下一回合。

| 指令 | 机制 |
| --- | --- |
| [`start_round`](instructions/start_round.md) | 检查回合上限、更新回合号并清空双方骰子。 |
| [`start_dice_roll_phase`](instructions/start_dice_roll_phase.md) | 开始并完整推进投掷阶段，包括准备事件、首次投骰和双方重投输入。 |
| [`start_battle`](instructions/start_battle.md) | 仅在第 1 回合广播战斗开始时机，后续回合为空操作。 |
| [`begin_action`](instructions/begin_action.md) | 广播行动阶段开始，提供行动选择，并处理切人与双方结束声明。 |
| [`end_round`](instructions/end_round.md) | 在双方结束声明结算完毕后关闭回合。 |

| Event | 机制 |
| --- | --- |
| [`dice_roll_preparation`](events/dice_roll_preparation.md) | 随机投骰前收集固定骰子和重投次数的工作区。 |
| [`battle_started`](events/battle_started.md) | 战斗开始时机。 |
| [`action_phase_started`](events/action_phase_started.md) | 行动阶段已经开始。 |
| [`before_action`](events/before_action.md) | 一名玩家即将选择行动。 |
| [`cost_of_switch`](events/cost_of_switch.md) | 计算主动切换出战角色的费用需求。 |
| [`dice_removed`](events/dice_removed.md) | 主动切人支付非零骰子费用后发出的通知。 |
| [`round_end_declared`](events/round_end_declared.md) | 当前行动玩家已宣布结束。 |
| [`round_ended`](events/round_ended.md) | 双方均已结束，本回合已经关闭。 |

## 伤害与元素附着

`deal_damage` 是完整伤害事务：它创建并推进 `damage_calculation`，进入 `damage_effect` 护盾等结算，扣除生命，处理伤害元素附着与反应，发布 `after_damage`，最后检查双方队伍是否全灭。`apply_element` 是完整元素附着事务：无反应时直接附着，有反应时推进 `elemental_reaction_will_occur`、默认或接管的反应效果，以及 `after_elemental_reaction`。

| 指令 | 机制 |
| --- | --- |
| [`deal_damage`](instructions/deal_damage.md) | 处理伤害计算、伤害结算、扣血、伤害元素附着、伤害后响应及胜负检查。 |
| [`absorb_damage_by_count`](instructions/absorb_damage_by_count.md) | 在 `damage_effect` 固定响应程序中，用当前实体计数吸收伤害。 |
| [`apply_element`](instructions/apply_element.md) | 完整处理元素附着、元素反应和反应后响应。 |
| [`set_element_aura`](instructions/set_element_aura.md) | 写入角色元素附着。 |

| Event | 机制 |
| --- | --- |
| [`damage_calculation`](events/damage_calculation.md) | 可修改伤害、倍率、类型和反应状态。 |
| [`damage_effect`](events/damage_effect.md) | 护盾等效果的结算工作区。 |
| [`after_damage`](events/after_damage.md) | 实际扣血已经完成，且伤害携带的元素附着/反应流程已经处理。 |
| [`elemental_reaction_will_occur`](events/elemental_reaction_will_occur.md) | 默认反应效果前的可接管工作区。 |
| [`after_elemental_reaction`](events/after_elemental_reaction.md) | 本次默认或接管的元素反应处理已结束。 |

## 调试

| 指令 | Event | 用途 |
| --- | --- | --- |
| [`test_command`](instructions/test_command.md) | [`test_event`](events/test_event.md) | 验证分派路径。 |
