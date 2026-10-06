[givm](../../reference.md) / [执行](../executor.md) / **handle_context**

# givm::handle_context

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<class TEntity, event_category Category = event_category::normal>
class handle_context;
```

一次事件响应使用的上下文，持有本次响应实体的只读视图。定义可以读取该实体及其所属牌桌、向指定定义查询规则信息，并提交同类别的后续效果及其全部输入。普通和立即上下文提供随机源；预览上下文不提供随机源，提交的效果留到确认操作后执行。

`normal_handle_context<TEntity>`、`immediate_handle_context<TEntity>`、`preview_handle_context<TEntity>` 是三类上下文的别名。

由执行器传给定义源的 `handle`，仅在本次响应调用期间有效。定义源不自行构造，也不得在响应结束后保存或使用本对象。

## 模板参数

| | |
| --- | --- |
| `TEntity` | 本次响应实体的只读 view 类型 |
| `Category` | 响应事件的类别；同时决定可提交的效果类型 |

## 成员函数

| | |
| --- | --- |
| [`entity`](handle_context/entity.md) | 取得当前响应实体 |
| [`table`](handle_context/table.md) | 读取当前牌桌 |
| [`query`](handle_context/query.md) | 向指定定义取得查询结果 |
| [`random`](handle_context/random.md) | 取得下一个随机值 |
| [`invoke`](handle_context/invoke.md) | 提交入口及其全部输入 |

## 参阅

| | |
| --- | --- |
| [定义源协议](../definition/source_protocol.md) | 事件响应与查询的定义方式 |
| [`normal_effect`](../definition/effect.md) | 已登记效果的入口 |
