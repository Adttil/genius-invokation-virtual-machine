[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **card_target_validation**

# givm::card_target_validation

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_target_validation;
```

请手牌自己的定义分步检查目标与其他用牌条件，并告知调用方能否完成或继续选择。

## 成员类型

| | |
| --- | --- |
| `result_t` | [`target_validation`](target_validation.md) |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | [`hand_card_view`](../../table/hand_card_view.md) | 准备打出的手牌。 |
| `table` | `const givm::table&` | 当前行动发生的牌桌。 |
| `library` | `const definition_library&` | 与牌桌配套的定义库，用于查询目标等实体的定义标签和其他定义信息。 |
| `targets` | `std::array<card_target_id, 2>` | 两个目标位置；未提供的位置为 `std::monostate`。 |
| `target_count` | `std::size_t` | 采用的目标数量：`0` 检查空选择，`1` 检查第一目标，`2` 检查第二目标。 |

## 注意

查询以只读参数对象传给定义源，直接返回检查结果，不修改事件、不返回后续效果入口，也不接收随机源。没有相应源查询时使用 [`query_default`](../query_default.md)：零目标返回 `valid_complete`，非零目标返回 `invalid`。

`target_count == 0` 可以表达必须选择目标、允许不选但也可继续选择，或者无需目标且不能继续选择，分别返回 `valid_incomplete`、`valid_complete_or_continue`、`valid_complete`。用牌条件不满足时仍可返回 `invalid`。

如果牌要求角色使用技能或主动特技，定义可通过 `library.is_controlled(character)` 检查该角色并拒绝受控时的使用。无需选择目标的牌也应在 `target_count == 0` 时检查这类用牌条件；不需要额外的卡牌分类标签。此查询只返回是否合法，实际效果程序不会自动补做控制检查。

只需一个目标的牌在第一目标有效时返回 `valid_complete`；需要两个目标时返回 `valid_incomplete`；第二目标可选时返回 `valid_complete_or_continue`。达到两个目标的上限后，查询应返回 `invalid` 或 `valid_complete`。

检查第二目标时，以第一目标合法为调用前提，查询可直接采用第一目标，不必重复检查，也不要求调用方实际调用过前一步。牌定义仍可拒绝不允许的目标数量。目标或相关对局条件改变后，调用方须重新保证第一目标合法。

[`card_targets_validate`](../../executor/execution_view/action_selection/card_targets_validate.md) 接收不定长目标 span，只采用前两个元素并补齐未提供的位置；后续元素忽略。直接构造本查询时，调用方保证 `target_count` 不大于 2，且所需目标位置均已填写。本查询不验证支付，也不提交出牌。
