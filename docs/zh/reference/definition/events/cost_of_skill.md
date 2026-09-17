[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **cost_of_skill**

# givm::cost_of_skill

定义于头文件 `<givm/definition.hpp>`

```cpp
struct cost_of_skill;
```

使用技能的费用计算事件。响应者可以调整所需骰子、充能与行动速度，并记录本次减费内容。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `skill` | `const skill_id` | 本次准备使用的技能；只读。 |
| `requirement` | [`action_cost_requirement`](action_cost_requirement.md) | 技能使用的骰子、充能费用与行动速度。 |
| `effect_argument` | `cost_effect_argument<cost_of_skill>` | 当前费用响应记录的减费内容。 |

## 注意

每次报价先读取定义库已保存的 [`skill_initial_cost`](../queries/skill_initial_cost.md) 结果作为基础费用，再依次处理费用响应。响应不得调用随机函数，违反此前提属于未定义行为。

需要在确认技能使用后执行的效果由响应返回的 [`handler_program_entry_t`](../handler_program_entry_t.md) 表达；其语境通过 [`handler_program_context_t`](../handler_program_context_t.md) 取得。报价时不会执行这些效果，确认后只执行所选技能的费用效果，且执行时机在扣除骰子与充能之前。

