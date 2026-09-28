[givm](../../reference.md) / [定义](../definition.md) / **source_selection_error**

# givm::source_selection_error

定义于头文件 `<givm/definition.hpp>`

```cpp
struct source_selection_error;
```

准备定义集合时找不到指定定义的诊断。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `definition` | [`definition_name`](definition_name.md) | 找不到的定义类别及名称 |
| `required_by` | `std::optional<definition_name>` | 需要该定义的源；直接选中的根定义不存在时为空 |
