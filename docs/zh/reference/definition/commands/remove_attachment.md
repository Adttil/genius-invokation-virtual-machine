[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_attachment**

# givm::remove_attachment

定义于头文件 `<givm/definition.hpp>`

使一个角色附属实体离场，并通知其他实体处理相应效果。

```cpp
struct remove_attachment_error;

struct remove_attachment
{
    using error_type = remove_attachment_error;

    using input_type = remove_attachment_input;

    relative_attachment_target target{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`remove_attachment_input`](../command_inputs/remove_attachment_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `remove_attachment_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `remove_attachment{}` 使用动态模式，由 `invoke` 提交一个 [remove_attachment_input](../command_inputs/remove_attachment_input.md)。
- 在 `target.selector` 中显式指定定义或装备类别时使用固定模式，不消费响应输入。按 `target.character` 的相对位置定位有效角色，不跳过生命值为零的角色，再选取此角色上首个同定义附属实体或当前指定类别装备。

动态输入可使用具体附属实体 ID，也可使用角色 ID 与装备类别，在命令执行时定位该角色当前的装备。两种模式均要求相应实体存在；完整定位规则见 [附属实体定位](attachment_target.md)。

## 结算

移除指定实体，再广播 [attachment_removed](../events/attachment_removed.md)，完整结算离场响应后继续下一条命令。实体离场后不再参与通常的遍历和广播；在 cleanup 前，其定义和状态仍可由旧 ID 读取。

## 编译检查

```cpp
struct remove_attachment_error;
```

`remove_attachment::error_type` 是 `givm::remove_attachment_error` 的别名。`remove_attachment_error` 是本命令的结构化编译错误，`remove_attachment_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_definition` | `target.selector` 的定义 ID 数值超出本次编译集合的 `attachment_view` 定义数量 |
| `invalid_equipment_type` | `target.selector` 不是有效装备类型，包括使用了 `equipment_type::none` |
| `invalid_target_character_player` | `target.character.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_target_character_selection` | `target.character.selection` 不是 `character_selection::character`；此处只允许单个角色 |

### `remove_attachment_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::uint64_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。
