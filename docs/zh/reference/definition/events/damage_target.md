[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_target**

# givm::damage_target

定义于头文件 `<givm/definition.hpp>`

[`damage`](damage.md) 用于定位角色的描述。既可以绑定具体角色，也可以在执行时相对出战位置定位一个角色；`damage::selection` 再决定命中该角色、其他角色或全部角色。

```cpp
using damage_target = std::variant<character_id, relative_character_target>;
```

| 分支 | 说明 |
| --- | --- |
| [`character_id`](../../table/character_id.md) | 一个具体角色 |
| [`relative_character_target`](relative_character_target.md) | 先按当前效果的本方或对方确定玩家，再按其当前出战位置循环偏移并寻找存活角色 |

两种分支都能配合 [`damage_target_selection`](../commands/damage_target_selection.md) 选择范围。需要命中多个独立的精确目标时，响应也可以向同一 [`deal_damage`](../commands/deal_damage.md) 提交多个 `damage` 输入。

每段初始描述先定位角色，再广播一次 `damage_preparation`，其中的 `target` 为可修改的 `character_id`。范围根据修饰后的角色展开；`damage_calculation`、`damage_effect` 和 `after_damage` 则分别针对每条具体伤害，`target` 都是只读 `character_id`。
