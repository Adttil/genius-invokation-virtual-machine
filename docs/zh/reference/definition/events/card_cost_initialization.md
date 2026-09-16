[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_cost_initialization**

# givm::card_cost_initialization

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_cost_initialization;
```

请手牌自己的定义准备基础出牌费用。在其他费用效果调整之前，牌定义可在这里给出骰子需求与行动速度。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次计算费用的手牌；只读。 |
| `requirement` | [`action_cost_requirement`](action_cost_requirement.md) | 基础费用，初始为零骰子的快速行动。 |

## 注意

每次重新报价都会重新准备基础费用。本事件仅调用本牌定义的响应；可打出的牌必须提供此响应，在默认需求上填写基础费用。

响应只修改事件并返回空入口，不执行后续效果。响应不得调用随机函数，违反此前提属于未定义行为。之后的费用调整通过 [`cost_of_card`](cost_of_card.md) 完成。
