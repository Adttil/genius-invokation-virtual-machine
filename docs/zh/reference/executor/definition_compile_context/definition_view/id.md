[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_compile_context](../../definition_compile_context.md) / [definition_view](../definition_view.md) / **id**

# givm::definition_compile_context::definition_view::id

定义于头文件 `<givm/definition_source.hpp>`

```cpp
definition_id<TCategory> id() const noexcept;
```

取得这项定义在本次编译中的 ID。

## 返回值

属于本次编译结果的 [`definition_id<TCategory>`](../../../table/definition_id.md)。ID 可以保存在编译后的定义配置中，不能跨不同定义库混用。
