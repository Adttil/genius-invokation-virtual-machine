[givm](../../reference.md) / [执行](../executor.md) / **compile_error_reason**

# givm::compile_error_reason

定义于头文件 `<givm/executor.hpp>`

```cpp
using compile_error_reason = std::variant</* 下表中的全部错误类型 */>;
```

一次编译错误的具体原因。全部错误类型直接位于同一层 variant，不再嵌套源准备或命令错误的 variant。

## 候选类型

| | |
| --- | --- |
| [`source_conflict`](../definition/source_conflict.md) | 基础定义与源集合的同名冲突 |
| [`source_missing_dependency`](../definition/source_missing_dependency.md) | 定义源缺少名称依赖 |
| [`source_selection_error`](../definition/source_selection_error.md) | 选择的定义或其依赖不存在 |
| [`definition_resolution_error`](definition_resolution_error.md) | 编译上下文的硬依赖解析错误 |
| [`definition_metadata_error`](definition_metadata_error.md) | 编译上下文的元数据查询 ID 无效或越界 |
| [`history_field_empty_name`](history_field_empty_name.md) | 历史字段名称为空 |
| [`history_field_duplicate_name`](history_field_duplicate_name.md) | 历史字段名称重复 |
| [`history_field_layout_overflow`](history_field_layout_overflow.md) | 单个历史字段布局溢出 |
| [`history_storage_layout_overflow`](history_storage_layout_overflow.md) | 全部历史摘要布局溢出 |
| [`history_field_access_error`](history_field_access_error.md) | 当前阶段不能取得历史字段 |
| [`history_field_not_found`](history_field_not_found.md) | 摘要中没有指定字段 |
| [`history_field_type_mismatch`](history_field_type_mismatch.md) | 字段数值类型或数组形态不符 |
| 每个[命令](../definition/commands.md)对应的 `givm::命令名_error` | 对应命令的参数错误，与该命令的 `error_type` 别名为同一类型 |

## 注意

命令错误的类型与命令一一对应。可以直接用 `std::get_if<givm::set_active_character_error>(&error.reason)` 检查切人命令的错误，也可沿用 `givm::set_active_character::error_type` 别名，不需要先取得一层通用命令错误。
