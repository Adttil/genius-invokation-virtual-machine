[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **add_combat_status**

# givm::add_combat_status

定义于头文件 `<givm/definition.hpp>`

直接添加一个出战状态。可用于定义希望独立保留多个同类效果，或在移除旧实体后创建新实体的情况。

```cpp
struct add_combat_status_error;

struct add_combat_status
{
    using error_type = add_combat_status_error;

    using input_type = add_combat_status_input;

    relative_player player = relative_player::self;
    definition_id<combat_status_view> definition{};
    combat_status_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`add_combat_status_input`](../command_inputs/add_combat_status_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `add_combat_status_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `add_combat_status{}` 使用动态模式，由 `invoke` 提交一个 [add_combat_status_input](../command_inputs/add_combat_status_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入明确指定目标玩家和定义，两者须合法。

## 结算

执行时读取 [combat_status_state_limit](../queries/combat_status_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建较少层数或次数的实体。

直接创建独立实体，同定义实体的存在不改变本次操作。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。

## 编译检查

```cpp
struct add_combat_status_error;
```

`add_combat_status::error_type` 是 `givm::add_combat_status_error` 的别名。`add_combat_status_error` 是本命令的结构化编译错误，`add_combat_status_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `combat_status_view` 定义数量 |

### `add_combat_status_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。
