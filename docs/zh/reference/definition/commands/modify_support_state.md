[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **modify_support_state**

# givm::modify_support_state

定义于头文件 `<givm/definition.hpp>`

按增量调整一个支援的状态。可表达消耗、增加和次数恢复。

```cpp
struct modify_support_state
{
    using input_type = modify_support_state_input;

    relative_player player = relative_player::self;
    definition_id<support_view> definition{};
    std::int64_t count{};
    std::int64_t round_usages{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`modify_support_state_input`](../command_inputs/modify_support_state_input.md)，动态模式下的输入类型 |

## 输入

- 默认构造 `modify_support_state{}` 使用动态模式，由 `invoke` 提交一个 [modify_support_state_input](../command_inputs/modify_support_state_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的支援中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入直接指定要操作的有效实体。

## 结算

执行到命令时，分别读取目标各字段的当前值，加上对应的有符号增量，再将结果限制在零与 [support_state_limit](../queries/support_state_limit.md) 对应字段之间。负增量表示消耗，正增量表示增加；`INT64_MIN`、`INT64_MAX` 也按这一规则处理，不发生算术回绕。

先写入新状态，再仅向被修改的实体发送 [support_state_changed](../events/support_state_changed.md)。层数或本回合次数为零时是否离场，由定义在此响应中决定。

通知包含修改前和裁剪后的状态；返回的响应程序完整结算后才继续下一条命令。

例如，动态输入 `modify_support_state_input{ .support = target, .round_usages = -1 }` 会在本命令实际执行时扣除一次可用次数；另一个字段的增量默认是零。
