[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **technique_target_id**

# givm::technique_target_id

定义于头文件 `<givm/definition.hpp>`

```cpp
using technique_target_id = variant_entity_id<entity_category::null, entity_category::character, entity_category::support, entity_category::summon>;
```

特技效果的目标标识。没有目标时保存 `null` 空类别，其余情况分别表示角色、支援或召唤物。
