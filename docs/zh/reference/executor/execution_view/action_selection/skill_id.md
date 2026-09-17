[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **skill_id**

# givm::execution_view<execution_state::action_selection>::skill_id

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr givm::skill_id skill_id(std::size_t skill_index) const noexcept;
```
[`skill_id`](../../../table/skill_id.md)

按技能候选索引取得对应的技能 ID，供上层查询牌桌并显示这项技能的信息。

## 参数

| | |
| --- | --- |
| `skill_index` | 从零开始的技能候选索引，须小于 [`skill_count()`](skill_count.md)。 |

## 返回值

指定技能候选的技能 ID。

## 注意

无需先计算费用。本操作不提交行动，也不修改牌桌或推进执行器。

候选索引仅用于当前行动现场。进入下一次行动现场后，须重新取得候选范围，不能假设同一索引仍指向原来的技能。

## 参阅

[`use_skill` 示例](use_skill.md#示例)演示技能查询、费用检查与提交行动。
