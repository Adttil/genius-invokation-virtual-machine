[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **reduce_combat_status_count**

# givm::reduce_combat_status_count

定义于头文件 `<givm/definition.hpp>`

```cpp
struct reduce_combat_status_count;
```

按提交的数量扣除指定出战状态的计数。状态与数量在响应时确定，执行本命令时才修改牌桌。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`combat_status_count_reduction`](../events/combat_status_count_reduction.md)，本命令消费的初始输入 |

## 注意

调用方须保证执行时该出战状态能够扣除指定数量。本命令不自动删除计数归零的状态，也不额外广播计数变化事件。

出战状态实现的护盾可在 `damage_effect` 响应中计算抵挡量并直接减少伤害，再通过 `context.invoke` 提交自身 ID 和抵挡量，由本命令扣除计数。命令本身不读取外层伤害事件或响应者。

## 参阅

| | |
| --- | --- |
| [`damage_effect`](../events/damage_effect.md) | 扣除生命前的伤害结算事件 |
| [`handle_context::invoke`](../../executor/handle_context/invoke.md) | 效果入口与输入的提交接口 |
