[givm](../../reference.md) / [执行](../executor.md) / **history_storage_layout_overflow**

# givm::history_storage_layout_overflow

定义于头文件 `<givm/compile.hpp>`

```cpp
struct history_storage_layout_overflow;
```

全部历史摘要所需空间无法用 `std::size_t` 表示的诊断。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `preceding_size` | `std::size_t` | 添加本摘要前已需的字节数 |
| `summary_size` | `std::size_t` | 本摘要所需的字节数 |
| `alignment` | `std::size_t` | 本摘要所需对齐 |
