[givm](../../reference.md) / [牌桌](../table.md) / **summon_definition_id**

# givm::summon_definition_id、givm::optional_summon_definition_id

定义于头文件 `<givm/table.hpp>`

```cpp
using summon_definition_id = definition_id<definition_category::summon>;
using optional_summon_definition_id = optional_definition_id<definition_category::summon>;
```

[`definition_id`](definition_id.md) 与 [`optional_definition_id`](optional_definition_id.md) 用于该定义类别的快捷别名。前者不能表示空值，后者允许尚未取得定义。
