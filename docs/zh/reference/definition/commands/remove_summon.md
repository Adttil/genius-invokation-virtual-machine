[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_summon**

# givm::remove_summon

定义于头文件 `<givm/definition.hpp>`

使一个召唤物离场，并通知其他实体处理相应效果。

```cpp
struct remove_summon
{
    using input_type = remove_summon_input;

    relative_player player = relative_player::self;
    definition_id<summon_view> definition{};
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`remove_summon_input`](../command_inputs/remove_summon_input.md)，动态模式下的输入类型 |

## 输入

- 默认构造 `remove_summon{}` 使用动态模式，由 `invoke` 提交一个 [remove_summon_input](../command_inputs/remove_summon_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的召唤物中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入直接指定要操作的有效实体。

## 结算

移除指定实体，再广播 [summon_removed](../events/summon_removed.md)，完整结算离场响应后继续下一条命令。实体离场后不再参与通常的遍历和广播；在 cleanup 前，其定义和状态仍可由旧 ID 读取。
