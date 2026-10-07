[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **summon**

# givm::summon

定义于头文件 `<givm/definition.hpp>`

发起一次召唤请求；已有同定义召唤物时，由它决定本次请求如何影响现有效果。

```cpp
struct summon_error;

struct summon
{
    using error_type = summon_error;

    using input_type = summon_input;

    relative_player player = relative_player::self;
    optional_definition_id<givm::definition_category::summon> definition{};
    summon_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`summon_input`](../command_inputs/summon_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `summon_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `summon{}` 使用动态模式，由 `invoke` 提交一个 [summon_input](../command_inputs/summon_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入明确指定目标玩家和定义，两者须合法。

## 结算

执行时读取 [summon_state_limit](../queries/summon_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建效果量或可用次数较少的召唤物。

在目标范围查找首个有效、定义 ID 相同的实体：

- 没有匹配实体时，目标玩家的有效召唤物数量小于其当前 [`player_state::summon_limit`](../../table/player_state.md) 才使用裁剪后的状态创建实体。达到或超过上限时，本次请求不创建实体，也不移除已有召唤物或广播 [summon_removed](../events/summon_removed.md)。
- 已有匹配实体时，仅向该实体发送 [this_summon_resummon](../events/this_summon_resummon.md)，携带本次裁剪后的状态；响应可从自身读取原有状态。即使召唤区已满或上限为零，仍会发送该事件。
- 返回的响应程序完整结算后，本命令结束。未提供响应或返回空入口时，不再追加实体。多个同定义实体也只通知首个。

创建新实体时，`usages` 可以为零，创建时不执行耗尽离场。

容量按命令实际执行时目标玩家的有效召唤物计数；已移除的实体和另一位玩家的召唤物不占用该玩家的容量。每位玩家的上限默认为 4。

已有实体的响应可以通过 [modify_summon_state](modify_summon_state.md)、[set_summon_state](set_summon_state.md)、[remove_summon](remove_summon.md) 或 [add_summon](add_summon.md) 表达累加、刷新、删除重建和独立创建。响应中的 `add_summon` 仍须按它实际执行时的容量判断是否创建新实体。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。

## 编译检查

```cpp
struct summon_error;
```

`summon::error_type` 是 `givm::summon_error` 的别名。`summon_error` 是本命令的结构化编译错误，`summon_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `summon_view` 定义数量 |

### `summon_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::uint64_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。
