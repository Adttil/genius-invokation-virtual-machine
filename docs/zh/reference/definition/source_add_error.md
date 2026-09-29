[givm](../../reference.md) / [定义](../definition.md) / **source_add_error**

# givm::source_add_error

定义于头文件 `<givm/source_library.hpp>`

```cpp
using source_add_error = std::variant<source_conflict, source_missing_dependency>;
```

登记单个或一批定义源时的诊断，包含 [`source_conflict`](source_conflict.md) 与 [`source_missing_dependency`](source_missing_dependency.md) 两种情况。

## 注意

[`definition_source_library::add`](definition_source_library/add.md) 或 [`make_definition_source_library`](make_definition_source_library.md) 失败时返回本类型的列表。可以通过 `std::visit` 或 `std::get_if` 读取结构化字段，或通过 [`error_string`](error_string.md) 取得可读文本。向已有库添加失败时，库保持不变。

合并源库只可能产生名称冲突，因此合并重载直接返回 `source_conflict` 列表。
