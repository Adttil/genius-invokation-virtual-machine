[givm](../../reference.md) / [定义](../definition.md) / **definition_name**

# givm::definition_name

定义于头文件 `<givm/definition.hpp>`

```cpp
struct definition_name;
```

由定义类别和名称组成的定义身份，用于源库登记诊断。同一个名称可以在不同定义类别中分别使用。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `category_index` | `std::size_t` | 类别在 [`definition_types`](definition_types.md) 中的索引 |
| `name` | `std::string` | 该类别内的定义名称 |

## 注意

类别索引从零开始，例如 `givm::definition_types::index_of<givm::card_definition>()` 表示卡牌定义。`name` 保存字符串副本，不借用源对象的名称存储。

## 参阅

| | |
| --- | --- |
| [`source_conflict`](source_conflict.md) | 同类别同名源的冲突诊断 |
| [`source_missing_dependency`](source_missing_dependency.md) | 定义源的缺失依赖诊断 |
