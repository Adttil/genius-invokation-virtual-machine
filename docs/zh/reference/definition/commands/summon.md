[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **summon**

# givm::summon

定义于头文件 `<givm/definition.hpp>`

发起一次召唤请求；已有同定义召唤物时，由它决定本次请求如何影响现有效果。

```cpp
struct summon
{
    using input_type = summon_input;

    relative_player player = relative_player::self;
    definition_id<summon_view> definition{};
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

## 输入

- 默认构造 `summon{}` 使用动态模式，由 `invoke` 提交一个 [summon_input](../command_inputs/summon_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入明确指定目标玩家和定义，两者须合法。

## 结算

执行时读取 [summon_state_limit](../queries/summon_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建效果量或可用次数较少的召唤物。

在目标范围查找首个有效、定义 ID 相同的实体：

- 没有匹配实体时，目标玩家的有效召唤物数量小于其当前 [`player_state::summon_limit`](../../table/player_state.md) 才使用裁剪后的状态创建实体。达到或超过上限时，本次请求不创建实体，也不移除已有召唤物或广播 [summon_removed](../events/summon_removed.md)。
- 已有匹配实体时，仅向该实体发送 [resummoning](../events/resummoning.md)，携带本次裁剪后的状态；响应可从自身读取原有状态。即使召唤区已满或上限为零，仍会发送该事件。
- 返回的响应程序完整结算后，本命令结束。未提供响应或返回空入口时，不再追加实体。多个同定义实体也只通知首个。

创建新实体时，`usages` 可以为零，创建时不发送状态修改通知。

容量按命令实际执行时目标玩家的有效召唤物计数；已移除的实体和另一位玩家的召唤物不占用该玩家的容量。每位玩家的上限默认为 4。

已有实体的响应可以通过 [modify_summon_state](modify_summon_state.md)、[set_summon_state](set_summon_state.md)、[remove_summon](remove_summon.md) 或 [add_summon](add_summon.md) 表达累加、刷新、删除重建和独立创建。响应中的 `add_summon` 仍须按它实际执行时的容量判断是否创建新实体。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。
