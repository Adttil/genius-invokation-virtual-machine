[givm](../../../reference.md) / [执行](../../executor.md) / [handle_context](../handle_context.md) / **query**

# givm::handle_context::query

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<definition_category TCategory, class TQuery>
requires requires { supported_queries<TCategory>::template index_of<TQuery>(); }
TQuery::result_t query(definition_id<TCategory> id, const TQuery& parameters) const;
```

在事件响应中向指定定义取得规则信息或检查结果。例如，减费圣遗物可向正在报价的天赋牌查询，它能否装备给圣遗物所属的角色。

## 模板参数

| | |
| --- | --- |
| `TCategory` | 由 ID 推导的定义类别。 |
| `TQuery` | 该类别的 [`supported_queries`](../../definition/supported_queries.md) 中的查询类型。 |

## 参数

| | |
| --- | --- |
| `id` | 当前对局定义库中的有效定义 ID。 |
| `parameters` | 查询所需的参数，须满足相应[查询类型](../../definition/queries.md)的前提。 |

## 返回值

`TQuery::result_t` 类型的查询结果。静态源未提供该查询，或动态源声明不支持时，使用 [`query_default`](../../definition/query_default.md) 的结果。

## 注意

查询语义与 [`definition_library::query`](../definition_library/query.md) 相同：空参数查询读取编译时保存的结果，其他查询使用本次参数求值，查询抛出的异常会传递给调用方。参数中的实体须来自当前牌桌，自身实体须采用 `id` 指定的定义。

查询不执行效果程序、不接收随机源，也不改变当前事件。是否查询及何时查询由响应自行决定；费用预览不会额外为每张牌调用装备对象查询。

## 参阅

| | |
| --- | --- |
| [`card_equipment_target_validation`](../../definition/queries/card_equipment_target_validation.md) | 卡牌对指定角色的装备适用性查询 |
