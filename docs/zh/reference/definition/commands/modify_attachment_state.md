[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **modify_attachment_state**

# givm::modify_attachment_state

定义于头文件 `<givm/definition.hpp>`

按增量调整一个角色附属实体的状态。可表达消耗、增加和次数恢复。

```cpp
struct modify_attachment_state_error;

struct modify_attachment_state
{
    using error_type = modify_attachment_state_error;

    using input_type = modify_attachment_state_input;

    relative_attachment_target target{};
    std::int64_t count{};
    std::int64_t round_usages{};
    bool ignore_limit = false;
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`modify_attachment_state_input`](../command_inputs/modify_attachment_state_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `modify_attachment_state_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `modify_attachment_state{}` 使用动态模式，由 `invoke` 提交一个 [modify_attachment_state_input](../command_inputs/modify_attachment_state_input.md)。
- 在 `target.selector` 中显式指定定义或装备类别时使用固定模式，不消费响应输入。按 `target.character` 的相对位置定位有效角色，不跳过生命值为零的角色，再选取此角色上首个同定义附属实体或当前指定类别装备。

动态输入可使用具体附属实体 ID，也可使用角色 ID 与装备类别，在命令执行时定位该角色当前的装备。两种模式均要求相应实体存在；完整定位规则见 [附属实体定位](attachment_target.md)。

`ignore_limit` 是命令选项，对固定和动态模式均生效，默认 `false`。动态输入不重复携带此选项；例如 `modify_attachment_state{ .ignore_limit = true }` 使用忽略定义上限的动态模式。

## 结算

执行到命令时，分别读取目标各字段的当前值 `v`，加上对应的有符号增量 `delta`。`ignore_limit == false` 时，结果限制在零与 `max(v, L)` 之间，其中 `L` 是 [attachment_state_limit](../queries/attachment_state_limit.md) 对应字段的默认上限。已经超限的字段不会继续增加，也不会因上限裁剪而减少；负增量正常扣除，零增量保持原值。

`ignore_limit == true` 时忽略定义上限，结果仅限制在零与 `UINT32_MAX` 之间。两种模式均支持 `INT64_MIN`、`INT64_MAX`，不发生算术回绕。

例如当前值为 3、定义上限为 2，普通模式增加 1 仍为 3，减少 1 得到 2；忽略上限时增加 1 得到 4。

先写入新状态，再仅向被修改的实体发送 [this_attachment_state_change](../events/this_attachment_state_change.md)。层数或本回合次数为零时是否离场，由定义在此响应中决定。

通知包含修改前和裁剪后的状态；返回的响应程序完整结算后才继续下一条命令。

例如，动态输入 `modify_attachment_state_input{ .attachment = target, .round_usages = -1 }` 会在本命令实际执行时扣除一次可用次数；另一个字段的增量默认是零。

## 编译检查

```cpp
struct modify_attachment_state_error;
```

`modify_attachment_state::error_type` 是 `givm::modify_attachment_state_error` 的别名。`modify_attachment_state_error` 是本命令的结构化编译错误，`modify_attachment_state_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_definition` | `target.selector` 的定义 ID 数值超出本次编译集合的 `attachment_view` 定义数量 |
| `invalid_equipment_type` | `target.selector` 不是有效装备类型，包括使用了 `equipment_type::none` |
| `invalid_target_character_player` | `target.character.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_target_character_selection` | `target.character.selection` 不是 `character_selection::character`；此处只允许单个角色 |

### `modify_attachment_state_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。
