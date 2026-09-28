[givm](../../reference.md) / [执行](../executor.md) / **history_field_type_mismatch**

# givm::history_field_type_mismatch

定义于头文件 `<givm/executor.hpp>`

```cpp
struct history_field_type_mismatch;
```

请求的历史字段类型与实际声明不一致的诊断，标量与数组也视为不同类型。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `summary` | `std::string` | 摘要名称 |
| `field` | `std::string` | 字段名称 |
| `expected_type` | `std::string` | 查询请求的类型，例如 `uint32_t` |
| `actual_type` | `std::string` | 字段实际声明的类型；数组带 `[]`，例如 `uint32_t[]` |
