[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_compile_context](../../definition_compile_context.md) / [definition_view](../definition_view.md) / **id**

# givm::definition_compile_context::definition_view::id

定义于头文件 `<givm/definition_source.hpp>`

```cpp
optional_definition_id<TCategory> id() const noexcept;
```

取得这项定义在本次编译中的 ID。

## 返回值

有效视图返回属于本次编译结果的 [`optional_definition_id<TCategory>`](../../../table/optional_definition_id.md)；诊断用空视图返回空值。取得其中的强类型 ID 后可以保存在编译后的定义配置中，不能跨不同定义库混用。
