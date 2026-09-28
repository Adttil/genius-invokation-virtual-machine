[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **generate_combat_status**

# givm::generate_combat_status

定义于头文件 `<givm/definition.hpp>`

生成出战状态；已有同定义实体时，由它决定本次请求如何影响现有效果。

```cpp
struct generate_combat_status_error;

struct generate_combat_status
{
    using error_type = generate_combat_status_error;

    using input_type = generate_combat_status_input;

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
| `input_type` | [`generate_combat_status_input`](../command_inputs/generate_combat_status_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `generate_combat_status_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `generate_combat_status{}` 使用动态模式，由 `invoke` 提交一个 [generate_combat_status_input](../command_inputs/generate_combat_status_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入明确指定目标玩家和定义，两者须合法。

## 结算

执行时读取 [combat_status_state_limit](../queries/combat_status_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建较少层数或次数的实体。

在目标范围查找首个有效、定义 ID 相同的实体：

- 没有匹配实体时，使用裁剪后的状态创建实体。
- 已有匹配实体时，仅向该实体发送 [combat_status_regeneration](../events/combat_status_regeneration.md)，携带本次裁剪后的状态；响应可从自身读取原有状态。
- 返回的响应程序完整结算后，本命令结束。未提供响应或返回空入口时，不再追加实体。多个同定义实体也只通知首个。

已有实体的响应可以通过 [modify_combat_status_state](modify_combat_status_state.md)、[set_combat_status_state](set_combat_status_state.md)、[remove_combat_status](remove_combat_status.md) 或 [add_combat_status](add_combat_status.md) 表达累加、刷新、删除重建和独立创建。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。

## 编译检查

```cpp
struct generate_combat_status_error;
```

`generate_combat_status::error_type` 是 `givm::generate_combat_status_error` 的别名。`generate_combat_status_error` 是本命令的结构化编译错误，`generate_combat_status_error::reason` 是原因枚举。[编译检查 `check`](../../executor/check.md) 使用本次定义集合与程序种类检查以下条件；[`compile`](../../executor/compile.md) 自动收集这些错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `combat_status_view` 定义数量 |

### `generate_combat_status_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。
