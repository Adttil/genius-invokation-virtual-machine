[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **modify_summon_state**

# givm::modify_summon_state

定义于头文件 `<givm/definition.hpp>`

按增量调整一个或多个召唤物的状态，可表达消耗、增加和次数恢复。

```cpp
struct modify_summon_state
{
    using input_type = modify_summon_state_input;

    relative_player player = relative_player::self;
    definition_id<summon_view> definition{};
    std::int64_t value{};
    std::int64_t usages{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`modify_summon_state_input`](../command_inputs/modify_summon_state_input.md)，动态模式下的输入类型 |

## 输入

- 默认构造 `modify_summon_state{}` 使用动态模式，由 `invoke` 提交一个 [modify_summon_state_input](../command_inputs/modify_summon_state_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的召唤物中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入以数组指定本次全部目标，可跨双方；允许为空，目标不得重复，并须在命令开始时有效。数组内容在 `invoke` 时复制。

## 结算

命令执行时读取全部目标的当前状态，分别加上 `value`、`usages` 的有符号增量，再逐字段饱和至零与 [summon_state_limit](../queries/summon_state_limit.md) 之间。`INT64_MIN`、`INT64_MAX` 也按此规则处理，不发生算术回绕。

全部目标完成状态写入之后，按输入顺序处理耗尽离场：若目标仍有效、定义具有 `remove_at_zero_usages` 标签且其当前 `usages == 0`，则移除并广播 [summon_removed](../events/summon_removed.md)，完整结算该通知后再处理下一项。标签可通过 [`definition_library::remove_at_zero_usages`](../../executor/definition_library/remove_at_zero_usages.md) 查询。

没有该标签的召唤物可以保留零次数。后续目标若被前面的离场响应删除，则跳过；若被恢复为非零次数，则不因本批修改离场。新产生的召唤物不加入本批。召唤物不接收状态修改自身通知。

例如 `modify_summon_state_input{ .summons = targets, .usages = -1 }` 会先对列表中所有目标各扣除一次，再逐个处理符合条件的离场；`value` 默认保持不变。
