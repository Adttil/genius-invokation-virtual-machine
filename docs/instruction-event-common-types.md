[返回](events.md)

# 指令、栈对象与公共类型

指令必须保持小型、平凡可复制且平凡析构，以便进入 executor 的擦除指令存储。指令是可保存的 re-entry point，不承载大型结算状态。现有代码目录和部分具体类型名仍沿用 command。

executor stack object 同样必须平凡可复制、平凡析构，且对齐不超过 `std::max_align_t`。stack object 的大小由对应 frame 协议决定；非平凡对象、局部引用、`std::span` 这类依赖外部生命周期的数据不得直接作为 stack object 保存。完整 stack 接口约定见 [Executor Stack](stack.md)。

若某个事件或输入需要变长数据，优先使用栈内动态数组 frame。若数据已经成为跨执行流程持久存在的游戏规则状态，应放入 `card_table`。

`random_fn` 是本次执行期间使用的非持有可调用引用，返回 `std::uint32_t`，不进入指令、table 或 executor stack。其擦除布局不属于公开接口，完整调用协议见 [随机输入](random-input.md)。

## 实体 ID 组合

系统不提供手写的 `entity_ref`。需要接受多类实体时，字段的完整类型如下：

| 含义 | 完整类型 |
| --- | --- |
| 事件描述的牌实体身份 | `std::variant<hand_card_id, deck_card_id>` |
| 任意运行期实体 | `std::variant<hand_card_id, deck_card_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` |
| 带计数器的实体 | `std::variant<support_id, summon_id, combat_status_id, attachment_id>` |
| 已解析的打牌目标 | `std::variant<std::monostate, character_id, support_id, summon_id>` |
| 伤害或元素附着来源 | `std::variant<hand_card_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` |

实现中，确实在多个字段表达同一含义的 variant 可以分别使用 `card_id`、`entity_id`、`counted_entity_id`、`card_target_id`、`effect_source_id`、`damage_source_id` 或 `element_application_source_id` 等便捷别名。这些别名不属于基础实体 ID，直接定义在首次使用它的 event 之前；不同语义的来源别名各自展开完整 variant，不通过另一个来源别名间接定义。

动态的“当前出战角色”“生命值最低角色”等选择规则不放进目标 ID。发起指令的固定逻辑先读取 `table` 并解析成具体 `character_id`，后续指令始终携带确定目标。

`hand_card_id` 与 `deck_card_id` 不可互换，因为区域是牌实体身份的一部分。`deck_card_id` 指向牌堆的稳定槽位；插牌或重排只修改单独的牌堆顺序，不改变 cleanup 前仍存活牌实体的 ID。牌被抽取或删除后，原 ID 立即只指向一个 invalid 槽位；`table.clean_up()` 会搬移存活槽位并收缩存储，此后所有此前保存的 `deck_card_id` 都不得继续使用。event 可以把 ID 作为同步通知中的当前或历史身份，指令不携带它作为延迟目标；延迟牌堆操作按定义、名称或标签搜索。弃牌会销毁牌实体，不产生弃牌区 ID。

## `dice_counts`

`dice_counts`、`elemental_dice_cost` 与 `elemental_dice` 一起定义在 `enums/elemental_dice.hpp`。`elemental_dice_cost` 目前仍是不完整的早期实现，本次只调整归属位置。`dice_counts` 在内部保存 8 个 `std::uint8_t`，但不暴露数组下标；只能使用 `elemental_dice` 索引：

```cpp
dice_counts dice;
dice[elemental_dice::pyro] = 2;
```

它表示玩家当前骰子、一次增减或玩家提交的具体支付骰子。`elemental_dice_cost` 表示费用约束，两者不可混用。

## 普通数值类型

普通规则数值直接使用定长整数，不再增加语义别名：

- 牌数量、伤害、充能、治疗、生命、秘传点和实体计数器使用 `std::uint32_t`。
- 一次资源变化的正负增量使用 `std::int32_t`。
- 骰子数量使用 `std::uint8_t`。

具体语义由字段名和所属指令/event 表达。

## 行动费用与行动输入

当前玩家行动窗口使用 `action_request` 作为行动输入槽。等待行动 frame 的物理栈顶是 `stage_t`，`action_request` 位于它前一个槽位，外层写输入时不改写 `stage_t`：

```cpp
struct action_request {
    action_request_kind request_kind;
    action_kind action_kind;
    stack_count_t action_index;
};
```

`request_kind` 区分仅计算费用、使用已有费用结果执行、现场计算并执行；对应名称为 `calculate_cost`、`do_action_with_cost` 和 `do_action`。`action_kind + action_index` 选择某类行动数组中的元素。真正执行时提交的骰子和目标放在独立的 `action_argument` 中：

```cpp
struct action_argument {
    dice_counts paid_dice;
    action_target target;
};
```

费用结果使用 `action_cost_requirement`：

```cpp
struct elemental_dice_requirement {
    dice_counts fixed;
    std::uint8_t same;
    std::uint8_t any;
};

struct action_cost_requirement {
    elemental_dice_requirement dice_requirement;
    action_speed speed;
};
```

`dice_requirement` 描述费用约束，`paid_dice` 描述外层实际选择支付的骰子，两者不可混用。当前最小实现只接通主动切人：`cost_of_switch` 是费用计算事件，默认费用为 1 个任意元素骰，默认速度为 `action_speed::combat`。

每个费用 handler 调用前，`begin_action` 清空事件中的 `cost_effect_argument<cost_of_switch>`。handler 可以修改费用、写入本次实际减费参数，并返回 `program_entry<onpay_context<cost_of_switch>>`。入口和参数共同保存为 `onpay_item<cost_of_switch>`；费用预览不执行 onpay，确认后只执行选中 action 对应的一行。

所有 onpay 完成后，`begin_action` 自己从 `action_argument.paid_dice` 扣除当前行动玩家的骰子，再继续 action 本体。费用合法性由外层根据对应 `cost_of_switch.requirement` 判断，核心消费流程假定输入已经合法。

## 伤害倍率

伤害指令/event 直接保存倍率字段，不再包装 `damage_multiplier`：

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `multiplier_numerator` | `std::uint16_t` | 分子，默认 1。 |
| `multiplier_denominator` | `std::uint16_t` | 分母，默认 1；核心假设拓展不会写入 0。 |

所有加算伤害先写入 `value`，倍率最后统一应用，避免响应顺序改变“先加算再倍乘”的规则。

## 规则枚举

规则枚举分别定义在 `include/givm/enums/` 下：

- `element`：七元素，以及表示“没有元素”的 `none`。
- `damage_type`：七元素伤害、物理、穿透和真实伤害。
- `damage_flag_bits` / `damage_flags`：伤害标记位及其强类型集合。
- `elemental_reaction`：元素反应类型。
- `element_application_cause`：普通效果附着或伤害附着。
- `action_speed`：快速行动或战斗行动。
- `skill_kind`：普通攻击、元素战技、元素爆发、被动或特技。
- `attachment_slot` / `weapon_type`：装备排斥槽与武器类型。
