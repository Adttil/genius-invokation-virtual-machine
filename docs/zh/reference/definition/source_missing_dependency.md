[givm](../../reference.md) / [定义](../definition.md) / **source_missing_dependency**

# givm::source_missing_dependency

定义于头文件 `<givm/definition.hpp>`

```cpp
struct source_missing_dependency;
```

登记定义源时，其按名称声明的依赖不在已有库或本批输入中的诊断。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`definition_name`](definition_name.md) | 声明依赖的定义源类别和名称 |
| `input_index` | `std::size_t` | 该源在本次 `add` 参数中的索引，从零开始 |
| `dependency` | [`definition_name`](definition_name.md) | 缺失依赖的类别和名称 |

## 注意

只检查按名称声明的依赖。编译中的标签筛选不声明或扩充依赖，也不会产生本诊断。重复输入同一源或重复声明同一依赖不会重复产生诊断；依赖名称已经发生名称冲突时，不再报告其缺失。

合并已登记的源库不重新检查依赖，因此合并重载不返回本诊断。

## 参阅

| | |
| --- | --- |
| [`definition_source_library::add`](definition_source_library/add.md) | 登记定义源或合并源库，并返回诊断 |
| [`source_add_error`](source_add_error.md) | 登记源时的诊断 variant |
| [`error_string`](error_string.md) | 将诊断列表转换为可读文本 |
