[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_target_check**

# givm::card_target_check

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_target_check;
```

请手牌自己的定义检查目标与其他用牌条件，例如目标是否为适用角色、当前对局是否满足这张牌的使用条件。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 准备打出的手牌；只读。 |
| `targets` | `const std::array<card_target_id, 2>` | 两个目标位置；只读。 |
| `result` | [`card_target_check_result`](card_target_check_result.md) | 检查结果，初始为 `valid`。 |

## 注意

本事件只调用本牌定义，可以省略响应；省略时结果为 `valid`。牌定义决定检查顺序与结果，未使用的目标位置忽略，库不检查目标数量。

响应只修改 `result` 并返回空入口，不执行后续效果；不得调用随机函数，违反此前提属于未定义行为。本检查不验证支付，也不提交出牌，由 [`check_card_targets`](../../executor/execution_view/action_selection/check_card_targets.md) 独立触发。

