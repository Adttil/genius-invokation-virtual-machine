[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_summon_state**

# givm::set_summon_state

定义于头文件 `<givm/definition.hpp>`

设置一个或多个召唤物的完整状态，用于指定效果量与剩余可用次数。

```cpp
struct set_summon_state
{
    using input_type = set_summon_state_input;

    relative_player player = relative_player::self;
    definition_id<summon_view> definition{};
    summon_state state{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`set_summon_state_input`](../command_inputs/set_summon_state_input.md)，动态模式下的输入类型 |

## 输入

- 默认构造 `set_summon_state{}` 使用动态模式，由 `invoke` 提交一个 [set_summon_state_input](../command_inputs/set_summon_state_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的召唤物中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入以数组指定本次全部目标，可跨双方；允许为空，目标不得重复，并须在命令开始时有效。数组内容在 `invoke` 时复制。

## 结算

按输入顺序，以每个目标定义的 [summon_state_limit](../queries/summon_state_limit.md) 逐字段裁剪后写入完整状态。固定模式使用命令的 `state`；动态模式的每项 `change` 都有自己的 `state`。

本命令不发送状态修改通知，也不触发耗尽离场，即使目标带 `remove_at_zero_usages` 标签且被设置为零次数也仍保留。需要强制移除时，后续显式执行 [remove_summon](remove_summon.md)。这允许先一次性设置全部指定召唤物，再开始处理离场通知。

例如清除召唤区时，可以先对需要归零的子集提交各自的 `{ 原效果量, 0 }`，再向 `remove_summon` 提交全部目标；其他召唤物保留原状态直到离场。
