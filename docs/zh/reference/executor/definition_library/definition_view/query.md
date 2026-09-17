[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **query**

# givm::definition_library::definition_view::query

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TQuery>
TQuery::result_t query(const TQuery& parameters) const;
```

取得当前定义的规则信息或检查结果。

## 模板参数

| | |
| --- | --- |
| `TQuery` | 当前定义类别的 [`supported_queries`](../../../definition/supported_queries.md) 中的查询类型。 |

## 参数

| | |
| --- | --- |
| `parameters` | 查询所需的参数，须满足相应[查询类型](../../../definition/queries.md)的前提。 |

## 返回值

`TQuery::result_t` 类型的查询结果；行为等同于 [`definition_library::query`](../query.md) 传入当前定义的 ID。

## 注意

空查询读取编译定义库时已保存的结果，非空查询使用本次参数求值。未提供源查询时使用 [`query_default`](../../../definition/query_default.md)。参数中的实体和牌桌须使用配套定义库，自身实体须采用当前定义。
