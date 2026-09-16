[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_target_check**

# givm::card_target_check

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_target_check;
```

请手牌自己的定义分步检查目标与其他用牌条件，并告知调用方能否完成或继续选择。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 准备打出的手牌；只读。 |
| `targets` | `const std::array<card_target_id, 2>` | 两个目标位置；未提供的位置为 `std::monostate`；只读。 |
| `target_count` | `const std::size_t` | 本次采用的目标数量：`0` 检查空选择，`1` 检查第一目标，`2` 检查第二目标；只读。 |
| `result` | [`card_target_check_result`](card_target_check_result.md) | 本步检查结果，初始为 `valid_complete`。 |

## 注意

本事件只调用本牌定义，可以省略响应；省略时结果为 `valid_complete`，忽略所给目标。提供响应时，`target_count == 0` 可以表达必须选择目标、允许不选但也可继续选择，或者无需目标且不能继续选择，分别返回 `valid_incomplete`、`valid_complete_or_continue`、`valid_complete`。用牌条件不满足时仍可返回 `invalid`。

只需一个目标的牌在第一目标有效时返回 `valid_complete`；需要两个目标时返回 `valid_incomplete`；第二目标可选时返回 `valid_complete_or_continue`。达到两个目标的上限后，响应应返回 `invalid` 或 `valid_complete`。

检查第二目标时，以第一目标合法为调用前提，响应可直接采用第一目标，不必重复检查，也不要求调用方实际调用过前一步。牌定义仍可拒绝不允许的目标数量。目标或相关对局条件改变后，调用方须重新保证第一目标合法。库不保存检查进度。

视图接收不定长目标 span，只将前两个元素放入 `targets`，并以采用的元素数量填写 `target_count`；后续元素忽略。

响应只修改 `result` 并返回空入口，不执行后续效果；不得调用随机函数，违反此前提属于未定义行为。本检查不验证支付，也不提交出牌，由 [`check_card_targets`](../../executor/execution_view/action_selection/check_card_targets.md) 独立触发。
