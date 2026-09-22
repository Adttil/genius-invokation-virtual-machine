[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **round_started**

# givm::round_started

定义于头文件 `<givm/definition.hpp>`

```cpp
struct round_started;
```

新回合开始的规则通知。响应者可在此处理回合开始效果，例如解除冻结。

## 注意

根回合流程先自动增加回合数、检查 [`game_parameters::max_rounds`](../../table/game_parameters.md) 并清空旧骰子。调用方应在回合程序中依次安排 [`start_dice_roll_phase`](../commands/start_dice_roll_phase.md) 和 [`start_round`](../commands/start_round.md)：前者完成双方投骰和全部重投，后者广播本事件。若回合数超限，对局在执行回合程序前结束。

本事件采用[全场广播](../events.md#全场广播)，候选响应者在广播开始时确定。各响应及其返回程序依次完成后，`start_round` 才结束。结束阶段产生且此时仍在场的冻结附属在本通知中解除，因此投骰阶段仍保持冻结。由本通知产生的额外骰子不参与已经完成的重投。

观察模式的 `execution_state::round_started` 由自动回合推进报告：当时只增加了回合数，尚未检查上限、清空骰子或执行回合程序。`start_round` 命令不会额外返回这个同名观察现场。投骰命令也不会隐式发送本规则事件。

## 参阅

| | |
| --- | --- |
| [冻结基础定义](../../basic_definitions.md) | 在本通知中解除冻结 |
| [`definition_library::is_controlled`](../../executor/definition_library/is_controlled.md) | 查询控制状态与回合开始解除示例 |
