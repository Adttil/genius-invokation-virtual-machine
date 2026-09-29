[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_combat_status**

# givm::remove_combat_status

定义于头文件 `<givm/definition.hpp>`

使一个出战状态离场，并通知其他实体处理相应效果。

```cpp
struct remove_combat_status_error;

struct remove_combat_status
{
    using error_type = remove_combat_status_error;

    using input_type = remove_combat_status_input;

    relative_player player = relative_player::self;
    definition_id<combat_status_view> definition{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`remove_combat_status_input`](../command_inputs/remove_combat_status_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `remove_combat_status_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `remove_combat_status{}` 使用动态模式，由 `invoke` 提交一个 [remove_combat_status_input](../command_inputs/remove_combat_status_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的出战状态中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入直接指定要操作的有效实体。

## 结算

移除指定实体，再广播 [combat_status_removed](../events/combat_status_removed.md)，完整结算离场响应后继续下一条命令。实体离场后不再参与通常的遍历和广播；在 cleanup 前，其定义和状态仍可由旧 ID 读取。

## 编译检查

```cpp
struct remove_combat_status_error;
```

`remove_combat_status::error_type` 是 `givm::remove_combat_status_error` 的别名。`remove_combat_status_error` 是本命令的结构化编译错误，`remove_combat_status_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `combat_status_view` 定义数量 |

### `remove_combat_status_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。
