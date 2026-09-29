[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_compile_context](../../definition_compile_context.md) / [definition_view](../definition_view.md) / **has_query**

# givm::definition_compile_context::definition_view::has_query

定义于头文件 `<givm/source.hpp>`

```cpp
template<class TQuery>
bool has_query() const noexcept;
```

检查定义是否为某项查询提供自己的实现。

## 模板参数

| | |
| --- | --- |
| `TQuery` | 要检查的查询类型 |

## 返回值

该定义启用了查询的自定义实现时返回 `true`；未提供、未启用或定义类别不支持该查询时返回 `false`。仅有 [`query_default`](../../../definition/query_default.md) 默认结果不算自定义查询。

## 注意

本函数不执行查询，也不返回查询结果。动态定义源是否启用自定义查询由其 `can_query` 声明决定。
