[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_combat_status_state**

# givm::set_combat_status_state

定义于头文件 `<givm/definition.hpp>`

设置一个出战状态的状态。用于指定层数与本回合剩余次数。

```cpp
struct set_combat_status_state_error;

struct set_combat_status_state
{
    using error_type = set_combat_status_state_error;

    using input_type = set_combat_status_state_input;

    relative_player player = relative_player::self;
    definition_id<combat_status_view> definition{};
    combat_status_state state{};
    bool ignore_limit = false;
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`set_combat_status_state_input`](../command_inputs/set_combat_status_state_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `set_combat_status_state_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `set_combat_status_state{}` 使用动态模式，由 `invoke` 提交一个 [set_combat_status_state_input](../command_inputs/set_combat_status_state_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的出战状态中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入直接指定要操作的有效实体。

`ignore_limit` 是命令选项，对固定和动态模式均生效，默认 `false`。动态输入不重复携带此选项；例如 `set_combat_status_state{ .ignore_limit = true }` 使用忽略定义上限的动态模式。

## 结算

执行时逐字段写入新状态。`ignore_limit == false` 时，每个字段的结果为 `min(requested, max(v, L))`，其中 `requested` 是要求设置的值，`v` 是当前值，`L` 是 [combat_status_state_limit](../queries/combat_status_state_limit.md) 对应字段的默认上限。允许保留已有的超限值，也允许明确将它设为更小的值。

`ignore_limit == true` 时直接采用提供的完整状态，不按定义上限裁剪。`state{}` 的两个字段均为零。

例如当前值为 3、定义上限为 2，普通模式设置为 3 或 4 均得到 3，设置为 2 得到 2。需要补足次数且保留原有次数时，定义应提交原值与补足目标中的较大者；直接提交较小的值表示明确降低。

先写入新状态，再仅向被修改的实体发送 [this_combat_status_state_change](../events/this_combat_status_state_change.md)。层数或本回合次数为零时是否离场，由定义在此响应中决定。

通知包含修改前和裁剪后的状态；返回的响应程序完整结算后才继续下一条命令。

## 编译检查

```cpp
struct set_combat_status_state_error;
```

`set_combat_status_state::error_type` 是 `givm::set_combat_status_state_error` 的别名。`set_combat_status_state_error` 是本命令的结构化编译错误，`set_combat_status_state_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `combat_status_view` 定义数量 |

### `set_combat_status_state_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。
