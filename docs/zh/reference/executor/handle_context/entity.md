[givm](../../../reference.md) / [执行](../../executor.md) / [handle_context](../handle_context.md) / **entity**

# givm::handle_context::entity

定义于头文件 `<givm/definition_source.hpp>`

```cpp
const TEntity& entity() const noexcept;
```

取得正在响应当前事件的实体，用于查看自身状态和所属玩家。

## 返回值

上下文持有的只读实体视图。类型由 [`handle_context<TEntity>`](../handle_context.md) 的模板参数确定。

## 注意

返回的引用只在本次响应上下文存活期间有效；实体视图本身不拥有牌桌。视图的存活要求见[实体的身份与访问](../../table/entity_access.md)。
