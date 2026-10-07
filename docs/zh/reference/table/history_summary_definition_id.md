[givm](../../reference.md) / [牌桌](../table.md) / **history_summary_definition_id**

# givm::history_summary_definition_id、givm::optional_history_summary_definition_id

定义于头文件 `<givm/table.hpp>`

```cpp
using history_summary_definition_id = definition_id<definition_category::history_summary>;
using optional_history_summary_definition_id = optional_definition_id<definition_category::history_summary>;
```

[`definition_id`](definition_id.md) 与 [`optional_definition_id`](optional_definition_id.md) 用于该定义类别的快捷别名。前者不能表示空值，后者允许尚未取得定义。
