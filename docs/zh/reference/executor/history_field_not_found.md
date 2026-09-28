[givm](../../reference.md) / [执行](../executor.md) / **history_field_not_found**

# givm::history_field_not_found

定义于头文件 `<givm/executor.hpp>`

```cpp
struct history_field_not_found;
```

目标历史摘要未声明请求字段的诊断。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `summary` | `std::string` | 摘要名称 |
| `field` | `std::string` | 请求的字段名称 |
