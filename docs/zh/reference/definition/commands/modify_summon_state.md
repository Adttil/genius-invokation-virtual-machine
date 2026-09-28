[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **modify_summon_state**

# givm::modify_summon_state

定义于头文件 `<givm/definition.hpp>`

按增量调整一个或多个召唤物的状态，可表达消耗、增加和次数恢复。

```cpp
struct modify_summon_state_error;

struct modify_summon_state
{
    using error_type = modify_summon_state_error;

    using input_type = modify_summon_state_input;

    relative_player player = relative_player::self;
    definition_id<summon_view> definition{};
    std::int64_t value{};
    std::int64_t usages{};
    bool ignore_limit = false;
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`modify_summon_state_input`](../command_inputs/modify_summon_state_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `modify_summon_state_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `modify_summon_state{}` 使用动态模式，由 `invoke` 提交一个 [modify_summon_state_input](../command_inputs/modify_summon_state_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的召唤物中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入以数组指定本次全部目标，可跨双方；允许为空，目标不得重复，并须在命令开始时有效。数组内容在 `invoke` 时复制。

`ignore_limit` 是命令选项，对固定和动态模式均生效，默认 `false`。动态输入不重复携带此选项；例如 `modify_summon_state{ .ignore_limit = true }` 使用忽略定义上限的动态模式。

## 结算

执行到命令时，分别读取目标各字段的当前值 `v`，加上对应的有符号增量 `delta`。`ignore_limit == false` 时，结果限制在零与 `max(v, L)` 之间，其中 `L` 是 [summon_state_limit](../queries/summon_state_limit.md) 对应字段的默认上限。已经超限的字段不会继续增加，也不会因上限裁剪而减少；负增量正常扣除，零增量保持原值。

`ignore_limit == true` 时忽略定义上限，结果仅限制在零与 `UINT32_MAX` 之间。两种模式均支持 `INT64_MIN`、`INT64_MAX`，不发生算术回绕。

例如当前值为 3、定义上限为 2，普通模式增加 1 仍为 3，减少 1 得到 2；忽略上限时增加 1 得到 4。

全部目标完成状态写入之后，按输入顺序处理耗尽离场：若目标仍有效、定义具有 `remove_at_zero_usages` 标签且其当前 `usages == 0`，则移除并广播 [summon_removed](../events/summon_removed.md)，完整结算该通知后再处理下一项。标签可通过 [`definition_library::remove_at_zero_usages`](../../executor/definition_library/remove_at_zero_usages.md) 查询。

没有该标签的召唤物可以保留零次数。后续目标若被前面的离场响应删除，则跳过；若被恢复为非零次数，则不因本批修改离场。新产生的召唤物不加入本批。召唤物不接收状态修改自身通知。

例如 `modify_summon_state_input{ .summons = targets, .usages = -1 }` 会先对列表中所有目标各扣除一次，再逐个处理符合条件的离场；`value` 默认保持不变。

## 编译检查

```cpp
struct modify_summon_state_error;
```

`modify_summon_state::error_type` 是 `givm::modify_summon_state_error` 的别名。`modify_summon_state_error` 是本命令的结构化编译错误，`modify_summon_state_error::reason` 是原因枚举。[编译检查 `check`](../../executor/check.md) 使用本次定义集合与程序种类检查以下条件；[`compile`](../../executor/compile.md) 自动收集这些错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `summon_view` 定义数量 |

### `modify_summon_state_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。
