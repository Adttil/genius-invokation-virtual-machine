[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **cost_of_skill**

# givm::cost_of_skill

定义于头文件 `<givm/definition.hpp>`

```cpp
struct cost_of_skill;
```

使用技能的费用计算事件。响应者可以调整所需骰子、充能与行动速度。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `skill` | `const skill_id` | 本次准备使用的技能；只读。 |
| `requirement` | [`action_cost_requirement`](../../table/action_cost_requirement.md) | 技能使用的骰子、充能费用与行动速度。 |

## 注意

同一行动窗口内，每个候选只能计算一次费用，之后可反复读取结果；库不检查重复计算。所有费用响应读取报价期间不变的牌桌，前一响应只通过费用事件影响后一响应；已提交效果不会在报价时修改牌桌。

报价先读取定义库已保存的 [`skill_initial_cost`](../queries/skill_initial_cost.md) 结果作为基础费用，再依次处理费用响应。响应不得调用随机函数，违反此前提属于未定义行为。

需要确认行动后执行的效果由响应通过 [`handle_context::invoke`](../../executor/handle_context/invoke.md) 提交，必须采用 `return context.invoke(givm::substack_t{}, entry, inputs...);` 的形式，没有输入时也须传这个标记。报价时只保留入口和输入，确认后在扣除骰子与充能之前执行所选候选的效果；误用普通重载属于未定义行为，不进行运行期检查。

