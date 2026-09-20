[givm](../../reference.md) / [牌桌](../table.md) / **card_state**

# givm::card_state

定义于头文件 `<givm/table.hpp>`

```cpp
struct card_state
{
    action_cost_requirement cost{ .speed = action_speed::fast };
    bool elemental_tuning_allowed = true;
};
```

一张卡牌自身的费用与元素调和许可。这些属性随卡牌保存在牌桌中；打出时的增减费另外计算，不会改写卡牌自身的费用。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cost` | [`action_cost_requirement`](action_cost_requirement.md) | 卡牌自身的费用与行动速度，默认零骰子、零充能的快速行动 |
| `elemental_tuning_allowed` | `bool` | 是否允许将这张牌用于元素调和，默认 `true` |

## 注意

初始属性由卡牌定义的 [`card_initial_state`](../definition/queries/card_initial_state.md) 给出。卡牌附属状态可通过 [`card_state_modification`](../definition/queries/card_state_modification.md) 修改这些属性；该查询只依赖定义和传入的卡牌、附属状态数据，不根据牌外的动态状态自动改变结果。

抽牌、换牌等区域转移保留当前属性。整张牌离场时也保留最后的属性，后续事件在清理前仍可通过原 ID 读取。

出牌报价先复制 `cost`，再进行 [`cost_of_card`](../definition/events/cost_of_card.md) 广播。减少本次支付费用与改变卡牌自身费用是两种不同的效果。
