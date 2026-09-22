[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **round_started**

# givm::round_started

定义于头文件 `<givm/definition.hpp>`

```cpp
struct round_started;
```

新回合开始的规则通知。响应者可在此处理回合开始效果，例如解除冻结。

## 注意

[`start_round`](../commands/start_round.md) 先增加回合数，检查回合上限；对局继续时清空双方元素骰，再广播本事件。若回合数已经超过上限，则直接判定双方失败，不清空骰子，也不广播本事件。

本事件采用[全场广播](../events.md#全场广播)，候选响应者在广播开始时确定。各响应及其返回程序依次完成后，`start_round` 才结束。结束阶段产生且此时仍在场的冻结附属可在本通知中解除。

观察模式的 `execution_state::round_started` 与本规则事件处于不同位置：观察现场只表示回合数已经增加，尚未检查上限、清空骰子或执行本事件。后续推进才处理这些步骤。掷骰子仍由独立的 [`start_dice_roll_phase`](../commands/start_dice_roll_phase.md) 安排。

## 参阅

| | |
| --- | --- |
| [冻结基础定义](../../basic_definitions.md) | 在本通知中解除冻结 |
| [`definition_library::is_controlled`](../../executor/definition_library/is_controlled.md) | 查询控制状态与回合开始解除示例 |
