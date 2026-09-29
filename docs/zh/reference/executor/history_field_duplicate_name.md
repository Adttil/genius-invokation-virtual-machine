[givm](../../reference.md) / [执行](../executor.md) / **history_field_duplicate_name**

# givm::history_field_duplicate_name

定义于头文件 `<givm/compile.hpp>`

```cpp
struct history_field_duplicate_name;
```

同一历史摘要中字段名称重复的诊断。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `field` | `std::string` | 重复名称 |
| `first_index` | `std::size_t` | 首次声明的下标，从零开始 |
| `repeated_index` | `std::size_t` | 本次重复声明的下标，从零开始 |
