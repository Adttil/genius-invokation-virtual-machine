[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **modify_support_state**

# givm::modify_support_state

定义于头文件 `<givm/definition.hpp>`

按增量调整一个支援的状态。可表达消耗、增加和次数恢复。

```cpp
struct modify_support_state_error;

struct modify_support_state
{
    using error_type = modify_support_state_error;

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
| [`error_type`](#编译检查) | `modify_support_state_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `modify_support_state{}` 使用动态模式，由 `invoke` 提交一个 [modify_support_state_input](../command_inputs/modify_support_state_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的支援中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入直接指定要操作的有效实体。

## 结算

执行到命令时，分别读取目标各字段的当前值，加上对应的有符号增量，再将结果限制在零与 [support_state_limit](../queries/support_state_limit.md) 对应字段之间。负增量表示消耗，正增量表示增加；`INT64_MIN`、`INT64_MAX` 也按这一规则处理，不发生算术回绕。

先写入新状态，再仅向被修改的实体发送 [support_state_changed](../events/support_state_changed.md)。层数或本回合次数为零时是否离场，由定义在此响应中决定。

通知包含修改前和裁剪后的状态；返回的响应程序完整结算后才继续下一条命令。

例如，动态输入 `modify_support_state_input{ .support = target, .round_usages = -1 }` 会在本命令实际执行时扣除一次可用次数；另一个字段的增量默认是零。

## 编译检查

```cpp
struct modify_support_state_error;
```

`modify_support_state::error_type` 是 `givm::modify_support_state_error` 的别名。`modify_support_state_error` 是本命令的结构化编译错误，`modify_support_state_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `support_view` 定义数量 |

### `modify_support_state_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。
