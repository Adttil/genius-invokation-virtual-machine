[givm](../../reference.md) / [定义](../definition.md) / **source_preparation_error**

# givm::source_preparation_error

定义于头文件 `<givm/source_library.hpp>`

```cpp
using source_preparation_error = std::variant<
    source_conflict, source_missing_dependency, source_selection_error>;
```

准备本次定义选择和 ID 映射时的诊断类型。

## 候选类型

| | |
| --- | --- |
| [`source_conflict`](source_conflict.md) | 基础源与已有集合的名称冲突 |
| [`source_missing_dependency`](source_missing_dependency.md) | 基础源的名称依赖缺失 |
| [`source_selection_error`](source_selection_error.md) | 选择根或其依赖不存在 |

## 参阅

| | |
| --- | --- |
| [`make_issued_id_map`](definition_source_library/make_issued_id_map.md) | 准备本次定义 ID |
| [`error_string`](error_string.md) | 格式化源准备诊断 |
