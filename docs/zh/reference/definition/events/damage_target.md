[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_target**

# givm::damage_target

定义于头文件 `<givm/definition.hpp>`

[`damage`](damage.md) 的目标描述。既可以绑定具体角色，也可以在执行时定位或展开目标。

```cpp
using damage_target = std::variant<character_id, relative_character_target, other_characters_target>;
```

| 分支 | 说明 |
| --- | --- |
| [`character_id`](../../table/character_id.md) | 一个具体角色 |
| [`relative_character_target`](relative_character_target.md) | 按指定一方的当前出战位置循环偏移并寻找存活角色 |
| [`other_characters_target`](other_characters_target.md) | 指定角色以外的同方存活角色 |

具体伤害开始广播时，范围已经展开为具体角色；`damage_preparation` 的 `target` 为可修改的 `character_id`。属性修饰结束后，`damage_calculation`、`damage_effect` 和 `after_damage` 的 `target` 都是只读 `character_id`。
