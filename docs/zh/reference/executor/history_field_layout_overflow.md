[givm](../../reference.md) / [执行](../executor.md) / **history_field_layout_overflow**

# givm::history_field_layout_overflow

定义于头文件 `<givm/executor.hpp>`

```cpp
struct history_field_layout_overflow;
```

历史字段所需空间无法用 `std::size_t` 表示的诊断。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `field` | `std::string` | 字段名称 |
| `field_index` | `std::size_t` | 字段声明的下标，从零开始 |
| `count` | `std::size_t` | 元素数量 |
| `element_size` | `std::size_t` | 单个元素的字节数 |
| `alignment` | `std::size_t` | 字段所需对齐 |
| `preceding_size` | `std::size_t` | 添加本字段前已需的字节数 |
