[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **attach**

# givm::attach

定义于头文件 `<givm/definition.hpp>`

向角色附属状态或装备；已有同定义实体时，由它决定本次请求如何影响现有效果。

```cpp
struct attach_error;

struct attach
{
    using error_type = attach_error;

    using input_type = attach_input;

    relative_player player = relative_player::self;
    definition_id<attachment_view> definition{};
    attachment_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`attach_input`](../command_inputs/attach_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `attach_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `attach{}` 使用动态模式，由 `invoke` 提交一个 [attach_input](../command_inputs/attach_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定一方执行到本命令时的出战角色。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入明确指定目标角色和定义，目标角色必须有效。固定模式的出战角色目标必须有效。

## 结算

若本次定义带 `control` 标签，且目标当前具有 `control_immunity` 附属，则忽略本次请求：不创建实体，也不调用已有同定义实体的重复施加响应。保护不移除已经存在的控制。查询保护可用 [`definition_library::is_control_immune`](../../executor/definition_library/is_control_immune.md)。

执行时读取 [attachment_state_limit](../queries/attachment_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建较少层数或次数的实体。

在目标范围查找首个有效、定义 ID 相同的实体：

- 没有匹配实体时，使用裁剪后的状态创建实体。装备替换遵守 [add_attachment](add_attachment.md) 的规则。
- 已有匹配实体时，仅向该实体发送 [this_attachment_reapply](../events/this_attachment_reapply.md)，携带本次裁剪后的状态；响应可从自身读取原有状态。
- 返回的响应程序完整结算后，本命令结束。未提供响应或返回空入口时，不再追加实体。多个同定义实体也只通知首个。

已有实体的响应可以通过 [modify_attachment_state](modify_attachment_state.md)、[set_attachment_state](set_attachment_state.md)、[remove_attachment](remove_attachment.md) 或 [add_attachment](add_attachment.md) 表达累加、刷新、删除重建和独立创建。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。

## 编译检查

```cpp
struct attach_error;
```

`attach::error_type` 是 `givm::attach_error` 的别名。`attach_error` 是本命令的结构化编译错误，`attach_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `attachment_view` 定义数量 |

### `attach_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。
