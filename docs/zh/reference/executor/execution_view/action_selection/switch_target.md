[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **switch_target**

# givm::execution_view<execution_state::action_selection>::switch_target

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr character_id switch_target(std::size_t target_index) const noexcept;
```
[`character_id`](../../../table/character_id.md)

按切换候选索引取得对应的角色 ID，供上层查询牌桌并显示这个角色的信息。

## 参数

| | |
| --- | --- |
| `target_index` | 从零开始的切换候选索引，须小于 [`switch_target_count()`](switch_target_count.md)。 |

## 返回值

指定切换候选的角色 ID。

## 注意

无需先计算费用。本操作不提交行动，也不修改牌桌或推进执行器。

候选索引仅用于当前行动现场。进入下一次行动现场后，须重新取得候选范围，不能假设同一索引仍指向原来的角色。

## 参阅

[`begin_action` 示例](../../../definition/commands/begin_action.md#示例)演示如何从候选索引取得 ID 并读取牌桌信息。
