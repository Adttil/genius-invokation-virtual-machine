[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **find_definition**

# givm::definition_compile_context::find_definition

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<class TCategory>
std::optional<definition_view<TCategory>> find_definition(std::string_view name) const;
```

按名称查找本次编译集合中的定义，适合根据本场规则中可用的内容选择效果。

## 模板参数

| | |
| --- | --- |
| `TCategory` | 要查询的定义类别 |

## 参数

| | |
| --- | --- |
| `name` | 定义名称 |

## 返回值

存在时返回 [`definition_view<TCategory>`](definition_view.md)；本次集合中不存在该名称时返回 `std::nullopt`。

## 注意

不要求名称依赖声明，也不会将未选择的定义加入编译集合。源库中存在但未进入本次集合的定义同样返回空结果。视图仅在本次编译期间有效。

定义必需的名称依赖应通过声明和 [`resolve_id`](resolve_id.md) 表达；本函数适用于允许不存在的查找。

## 参阅

| | |
| --- | --- |
| [`definitions`](definitions.md) | 遍历同类别的编译元数据，并给出完整示例 |
