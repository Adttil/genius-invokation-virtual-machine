[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_summon_state**

# givm::set_summon_state

定义于头文件 `<givm/definition.hpp>`

设置一个召唤物的状态。用于指定效果量与剩余可用次数。

```cpp
struct set_summon_state
{
    relative_player player = relative_player::self;
    definition_id<summon_view> definition{};
    summon_state state{};
};
```

## 输入

- 默认构造 `set_summon_state{}` 使用动态模式，由 `invoke` 提交一个 [summon_state_change](../events/summon_state_change.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的召唤物中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入直接指定要操作的有效实体。

## 结算

执行时读取 [summon_state_limit](../queries/summon_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。然后以裁剪结果替换目标的完整状态。`state{}` 的两个字段均为零。

先写入新状态，再仅向该召唤物发送 [summon_state_changed](../events/summon_state_changed.md)，`usages == 0` 时也一样。是否离场由召唤物自己的响应决定；需要离场时，响应程序可执行 [remove_summon](remove_summon.md)，由该命令广播 [summon_removed](../events/summon_removed.md)。

通知包含修改前和裁剪后的状态；返回的响应程序完整结算后才继续下一条命令。
