[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_summon**

# givm::remove_summon

定义于头文件 `<givm/definition.hpp>`

使一个或多个召唤物依次离场，并通知其他实体处理相应效果。

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

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入以数组指定本次全部目标，可跨双方；允许为空，目标不得重复，并须在命令开始时有效。数组内容在 `invoke` 时复制。

## 结算

按目标顺序，先移除一个召唤物，再广播 [summon_removed](../events/summon_removed.md)，完整结算所有响应后继续下一目标。与批量舍弃手牌不同，本命令不会预先将全部目标标记离场。

命令不清空可用次数，也不改变效果量，不检查 `remove_at_zero_usages` 标签。目标的离场状态就是移除时的当前状态；需要统一归零等处理时，应在本命令之前显式安排 [set_summon_state](set_summon_state.md)。

前面响应已移除的后续目标跳过；新产生的召唤物不加入本批。离场后不再参与通常遍历和广播，但在 cleanup 前，其定义和状态仍可由旧 ID 读取。空目标列表无效果。
