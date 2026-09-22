[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **technique_target_id**

# givm::technique_target_id

定义于头文件 `<givm/definition.hpp>`

```cpp
using technique_target_id = std::variant<std::monostate, character_id, support_id, summon_id>;
```

特技效果的目标标识。没有目标时保存 std::monostate，其余情况分别表示角色、支援或召唤物。
