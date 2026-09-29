[givm](../../reference.md) / [执行](../executor.md) / **definition_metadata_error**

# givm::definition_metadata_error

定义于头文件 `<givm/compile.hpp>`

```cpp
struct definition_metadata_error;
```

编译上下文按无效或越界定义 ID 读取元数据时的错误。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `category_index` | `std::size_t` | 待查询类别在 [`definition_types`](../definition/definition_types.md) 中的索引 |
| `value` | `std::size_t` | 输入定义 ID 的数值 |
| `count` | `std::size_t` | 本次编译集合中该类别的定义数量，即有效 ID 数值范围的上界（不含） |

## 参阅

| | |
| --- | --- |
| [`definition_compile_context::operator[]`](definition_compile_context/operator_at.md) | 按 ID 读取定义元数据 |
