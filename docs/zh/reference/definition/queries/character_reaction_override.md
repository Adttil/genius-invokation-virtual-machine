[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **character_reaction_override**

# givm::character_reaction_override

```cpp
struct character_reaction_override { const elemental_reaction slot; };
```

装载牌组时查询角色对一个反应槽位的整局替换，返回 `optional_definition_id<givm::definition_category::reaction>`；默认返回空 ID，表示沿用默认反应。角色须声明 `reaction_dependencies()`，编译时按名称解析所需定义。

装载按牌组角色顺序应用非空替换；同槽位有多个替换时，后面的覆盖前面的。空 ID 不覆盖已有映射。替换在装载时确定，角色死亡不撤销它。
