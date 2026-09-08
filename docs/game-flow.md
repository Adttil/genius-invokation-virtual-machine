# 最小可运行对局

核心不预设固定的游戏流程。编译调用方使用核心公开指令集提供初始化程序和回合程序；它们与 definition 的响应程序共同构成游戏规则程序。玩家操作合法性由上层判断，核心指令假定收到合法输入。

本页给出一种七圣召唤式组合。每条指令的准确字段、广播顺序、挂起点和 stack ABI 以对应指令页面为准。

## 对局前准备

牌组是每局输入，不编入游戏流程程序。上层先用编译结果中的 `id_map` 把按名称保存的牌组链接成 `linked_deck`，再通过 `card_table::load_deck(...)` 将双方牌组装入 table。装载只创建默认的牌堆和角色数据，不使用随机数，也不执行角色 definition。

牌组链接、生命周期和装载接口见 [牌组链接与装载](deck.md)。

## 初始化程序

初始化程序可以依次包含：

1. 对双方执行 [`shuffle_deck`](instructions/shuffle_deck.md)。
2. 对双方执行 [`initialize_characters`](instructions/initialize_characters.md)。
3. 把初始先手映射为玩家 0，并设置 `table.state().active_player`。
4. 执行两条 [`draw_cards`](instructions/draw_cards.md)，分别为双方抽取初始手牌。
5. 执行 [`replace_cards_both`](instructions/replace_cards_both.md)，等待并结算双方换牌。
6. 执行 [`select_active_character_both`](instructions/select_active_character_both.md)，等待双方选择初始出战角色。

初始化程序只执行一次。是否洗牌、角色初始化顺序、初始手牌数和是否允许换牌都由调用方提供的指令序列决定；具体牌组内容来自对局前装入 table 的输入。

## 回合程序

当前默认形态可以写成：

```cpp
const auto round = std::tuple{
    start_round{ .max_rounds = 14 },
    start_dice_roll_phase{ .count = 8, .reroll_count = { 1, 1 } },
    start_battle{},
    begin_action{},
    end_round{},
    draw_cards{ .count = 2, .player = relative_player::current },
    draw_cards{ .count = 2, .player = relative_player::other }
};
```

`start_battle` 只在首回合广播战斗开始。`begin_action` 首次进入时广播行动阶段开始，并在同一条指令内反复提供行动机会；双方均宣告结束后进入 `end_round`。`end_round` 切换下一回合先手并广播回合结束，后续两条显式 `draw_cards` 完成双方抽牌。

回合程序结束后会重新从其开头执行。调用方只需描述初始化程序和回合程序，不需要额外维护循环状态；上层观察到的当前指令始终是公开规则步骤。

## 编译与启动

```cpp
const auto [library, id_map] = source_library.compile(initialization, round);
const auto deck = link_deck(id_map, card_names, character_names);
card_table table{ library };
table.load_deck(player_id{ 0 }, deck);
table.load_deck(player_id{ 1 }, deck);
executor execution;
execution.enter_entry(library);
```

`library` 及其 definition source 必须覆盖 table 的使用期。牌组链接完成后，`id_map` 不参与对局运行。`enter_entry(library)` 清空原有执行栈，并进入该 definition library 的游戏主流程入口。executor 不保存规则库引用；后续推进时使用的 table 必须引用同一个 definition library。运行到输入、观察挂起点或终局的通用循环见 [Table、Executor 与外层观察](table-vm-and-input.md)。随机输入与回放见 [随机输入](random-input.md)。

## 流程状态

table 不保存通用阶段枚举。当前流程由执行位置、当前指令及其 stack ABI 表达：

- 投骰等待由 `start_dice_roll_phase` 及其栈上 selector、重投状态和预发随机池表达；
- 当前行动方由 `table.state().active_player` 表达；
- `first_ended` 只表示本回合是否已有玩家宣告结束；
- 行动等待由 `begin_action` 的 `action_argument` 与 `action_request` 输入槽表达。

在 [`round_end_declared`](events/round_end_declared.md) 广播期间，`active_player` 仍是宣告者。第一次广播完成后，`begin_action` 把行动权交给对手；第二次广播完成后，`end_round` 把 `active_player` 切回首个结束玩家，并清空 `first_ended`。

`start_round` 在超过回合上限时以 `both_loss` 结束对局。所有可能降低生命的领域指令负责在相应事务完成后判断胜负。上层只通过 `executor.status()` 读取结果，不依赖其内部程序表示。

## 配置边界

游戏流程规则由初始化程序、回合程序及其运行语义配置，而不是 table 中的专用流程参数：

| 指令字段 | 示例值 | 含义 |
| --- | ---: | --- |
| `start_round::max_rounds` | 14 | 允许开始的最大回合编号。 |
| `start_dice_roll_phase::count` | 8 | 每回合初次投掷生成的骰子数。 |
| `start_dice_roll_phase::reroll_count` | `{1, 1}` | 双方默认重投次数。 |
| `draw_cards::count` | 2 | 该流程槽请求的抽牌数。 |
| `draw_cards::player` | `current` 或 `other` | 相对当前 `active_player` 的抽牌方。 |

`game_parameters` 只保留牌桌容量类规则：

| 字段 | 默认值 | 含义 |
| --- | ---: | --- |
| `hand_limit` | 10 | 有效手牌上限。 |
| `support_limit` | 4 | 支援区数量上限。 |
| `summon_limit` | 4 | 召唤物区数量上限。 |

[返回当前指令与事件目录](events.md)
